#include "SSP_Subsystem.h"
#include "FGSplineMeshGenerationLibrary.h"
#include <Kismet/GameplayStatics.h>
#include "SSP_RemoteCallObject.h"
#include "FGPlayerController.h"

#pragma optimize("", off)

ASSP_Subsystem::ASSP_Subsystem() : Super(), splinePathMesh(nullptr), mantaPathMesh(nullptr)
{
	//Spawn on client will help us catch exceptions where the server tries to spawn the path mesh components itself.
	//We don't want that as other players should not be bothered with our path visualization.
	ReplicationPolicy = ESubsystemReplicationPolicy::SpawnLocal;
}

bool ASSP_Subsystem::GetHasAuthority()
{
	return HasAuthority();
}

bool ASSP_Subsystem::ShouldSave_Implementation() const
{
	return true;
}

USplineMeshComponent* SplineMeshConstructor(USplineComponent* spline)
{
	USplineMeshComponent* newComponent = NewObject<USplineMeshComponent>(
		spline, USplineMeshComponent::StaticClass());
	newComponent->SetupAttachment(spline);
	newComponent->Mobility = EComponentMobility::Static;
	
	// Ensure proper setup for multiplayer
	newComponent->SetVisibility(true);
	newComponent->SetHiddenInGame(false);
	newComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	return newComponent;
}

TArray<USplineMeshComponent*> BuildSplineMeshes(
	class USplineComponent* spline,
	float maxSplineLengthToFill,
	UStaticMesh* mesh,
	float meshLength,
	bool makeMovable = false)
{
	TArray<USplineMeshComponent*> createdMeshes;
	
	const float splineLengthToFill = FMath::Clamp(spline->GetSplineLength(), 0.f, maxSplineLengthToFill);
	const int32 numMeshes = splineLengthToFill > SMALL_NUMBER
		                        ? FMath::Max(1, FMath::RoundToInt(splineLengthToFill / meshLength))
		                        : 0;

	// Create all required meshes directly
	for (int32 i = 0; i < numMeshes; ++i)
	{
		if (auto newMesh = SplineMeshConstructor(spline))
		{
			if (makeMovable)
			{
				newMesh->SetMobility(EComponentMobility::Movable);
			}
			createdMeshes.Push(newMesh);
		}
	}

	// Put all pieces along the spline.
	{
		const float segmentLength = splineLengthToFill / numMeshes;

		for (int32 i = 0; i < createdMeshes.Num(); ++i)
		{
			const float startDistance = (float)i * segmentLength;
			const float endDistance = (float)(i + 1) * segmentLength;
			const FVector startPos = spline->GetLocationAtDistanceAlongSpline(
				startDistance, ESplineCoordinateSpace::Local);
			const FVector startTangent = spline->GetTangentAtDistanceAlongSpline(
				startDistance, ESplineCoordinateSpace::Local).GetSafeNormal() * segmentLength;
			const FVector endPos = spline->GetLocationAtDistanceAlongSpline(endDistance, ESplineCoordinateSpace::Local);
			const FVector endTangent = spline->GetTangentAtDistanceAlongSpline(
				endDistance, ESplineCoordinateSpace::Local).GetSafeNormal() * segmentLength;

			createdMeshes[i]->SetStartAndEnd(startPos, startTangent, endPos, endTangent, true);
			createdMeshes[i]->SetStaticMesh(mesh);
		}
	}

	// Register new meshes, needs to happen after the properties are set for static components.
	for (auto meshComp : createdMeshes)
	{
		if (!meshComp->IsRegistered())
		{
			meshComp->RegisterComponent();
		}
		
		// Ensure visibility in multiplayer - set after registration
		meshComp->SetVisibility(true);
		meshComp->SetHiddenInGame(false);
		meshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		// Force update for multiplayer
		meshComp->MarkRenderStateDirty();
	}
	
	return createdMeshes;
}

void ASSP_Subsystem::HandlePathSplines(AFGDrivingTargetList* targetList, bool show)
{
	if (targetList && !bIsUpdatingPathVisibility)
	{
		bIsUpdatingPathVisibility = true;

		if (show)
		{
			// Clean up existing vehicle meshes first
			for (auto meshComp : VehicleMeshes)
			{
				if (meshComp && meshComp->IsRegistered())
				{
					meshComp->UnregisterComponent();
				}
			}
			VehicleMeshes.Empty();

			targetList->SetPathVisible(true);

			// Show splines using the existing path
			auto spline = targetList->GetPath();
			if (spline)
			{
				TArray<USplineMeshComponent*> createdMeshes = BuildSplineMeshes(spline, spline->GetSplineLength(), splinePathMesh, 100);
				VehicleMeshes.Append(createdMeshes);
			}
			else
			{
				// For multiplayer, try to create the path if we have authority
				bool bHasAuthority = GetHasAuthority();
				if (bHasAuthority)
				{
					targetList->CreatePath();
					spline = targetList->GetPath();
					if (spline)
					{
						TArray<USplineMeshComponent*> createdMeshes = BuildSplineMeshes(spline, spline->GetSplineLength(), splinePathMesh, 100);
						VehicleMeshes.Append(createdMeshes);
					}
				}
				else
				{
					// For clients, wait for the path to be replicated
					// Use a single retry with a longer delay
					if (targetList && targetList->GetWorld())
					{
						FTimerHandle RetryTimer;
						targetList->GetWorld()->GetTimerManager().SetTimer(RetryTimer, [this, targetList]()
							{
								auto spline = targetList->GetPath();
								if (spline)
								{
									TArray<USplineMeshComponent*> createdMeshes = BuildSplineMeshes(spline, spline->GetSplineLength(), splinePathMesh, 100);
									VehicleMeshes.Append(createdMeshes);
								}
							}, 1.0f, false); // Wait 1 second for replication
					}
				}
			}
		}
		else
		{
			targetList->SetPathVisible(false);

			// Clean up vehicle meshes when hiding paths
			for (auto meshComp : VehicleMeshes)
			{
				if (meshComp && meshComp->IsRegistered())
				{
					meshComp->UnregisterComponent();
				}
			}
			VehicleMeshes.Empty();
		}

		bIsUpdatingPathVisibility = false;
	}
}

void ASSP_Subsystem::ShowTargetPath(AFGTargetPoint* targetPoint)
{
	auto targetList = targetPoint->GetOwningList();
	if (targetList)
	{
		HandlePathSplines(targetList, true);
	}
}

void ASSP_Subsystem::HideTargetPath(AFGTargetPoint* targetPoint)
{
	auto targetList = targetPoint->GetOwningList();
	if (targetList)
	{
		HandlePathSplines(targetList, false);
	}
}

void ASSP_Subsystem::ToggleVehiclePath(AFGWheeledVehicle* vehicle)
{
	auto targetList = vehicle->GetTargetList();
	if (targetList)
	{
		HandlePathSplines(targetList, !targetList->IsPathVisible());
	}
}

void ASSP_Subsystem::ShowInitialPaths(AActor* actor)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(actor->GetWorld(), AFGDrivingTargetList::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		for (AActor* actor1 : FoundActors)
		{
			auto tpList = Cast<AFGDrivingTargetList>(actor1);
			if (tpList && tpList->mIsPathVisible)
			{
				HandlePathSplines(tpList, true);
			}
		}
	}
}

void ASSP_Subsystem::ShowAllMantaPaths(AActor* actor)
{
	// Clean up existing manta meshes first
	for (auto meshComp : MantaMeshes)
	{
		if (meshComp && meshComp->IsRegistered())
		{
			meshComp->UnregisterComponent();
		}
	}
	MantaMeshes.Empty();

	// Check if mantaPathMesh is null
	if (!mantaPathMesh)
	{
		return;
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(actor->GetWorld(), AFGManta::StaticClass(), FoundActors);
	
	if (FoundActors.Num() > 0)
	{
		for (auto actor1 : FoundActors)
		{
			auto manta = Cast<AFGManta>(actor1);
			if (manta)
			{
				auto spline = manta->mCachedSpline;
				if (spline)
				{
					TArray<USplineMeshComponent*> createdMeshes = BuildSplineMeshes(spline, spline->GetSplineLength(), mantaPathMesh, 2500, true);
					MantaMeshes.Append(createdMeshes);
				}
			}
		}
	}
}

void ASSP_Subsystem::HideAllMantaPaths(AActor* actor)
{
	// Clean up manta meshes when hiding manta paths
	for (auto meshComp : MantaMeshes)
	{
		if (meshComp && meshComp->IsRegistered())
		{
			meshComp->UnregisterComponent();
		}
	}
	MantaMeshes.Empty();
}

void ASSP_Subsystem::ShowAllVehiclePaths(AActor* actor)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(actor->GetWorld(), AFGDrivingTargetList::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		for (auto actor1 : FoundActors)
		{
			auto tpList = Cast<AFGDrivingTargetList>(actor1);
			tpList->SetPathVisible(true);
			HandlePathSplines(tpList, tpList->mIsPathVisible);
		}
	}
}

void ASSP_Subsystem::HideAllVehiclePaths(AActor* actor)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(actor->GetWorld(), AFGDrivingTargetList::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		for (auto actor1 : FoundActors)
		{
			auto tpList = Cast<AFGDrivingTargetList>(actor1);
			tpList->SetPathVisible(false);
			HandlePathSplines(tpList, tpList->mIsPathVisible);
		}
	}
}
#pragma optimize("", on)
