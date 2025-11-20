#include "SSP_Subsystem.h"
#include "FGSplineMeshGenerationLibrary.h"
#include <Kismet/GameplayStatics.h>
#include "SSP_RemoteCallObject.h"
#include "FGPlayerController.h"
#include "InstancedSplineMeshComponent.h"

#pragma optimize("", off)

ASSP_Subsystem::ASSP_Subsystem() : Super(), splinePathMesh(nullptr), mantaPathMesh(nullptr)
{
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

UInstancedSplineMeshComponent* CreateInstancedSplineMeshComponent(AActor* outer, USceneComponent* attachTo, UStaticMesh* mesh, bool makeMovable = false)
{
	UInstancedSplineMeshComponent* newComponent = NewObject<UInstancedSplineMeshComponent>(
		outer, UInstancedSplineMeshComponent::StaticClass());
	
	if (attachTo)
	{
		newComponent->SetupAttachment(attachTo);
	}
	else if (outer)
	{
		newComponent->SetupAttachment(outer->GetRootComponent());
	}
	
	newComponent->Mobility = makeMovable ? EComponentMobility::Movable : EComponentMobility::Static;
	
	newComponent->SetStaticMesh(mesh);
	
	newComponent->SetVisibility(true);
	newComponent->SetHiddenInGame(false);
	newComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	if (!newComponent->IsRegistered())
	{
		newComponent->RegisterComponent();
	}
	
	return newComponent;
}

UInstancedSplineMeshComponent* BuildSplineMeshes(
	class USplineComponent* spline,
	float maxSplineLengthToFill,
	UStaticMesh* mesh,
	float meshLength,
	bool makeMovable = false)
{
	const float splineLengthToFill = FMath::Clamp(spline->GetSplineLength(), 0.f, maxSplineLengthToFill);
	const int32 numMeshes = splineLengthToFill > SMALL_NUMBER
		                        ? FMath::Max(1, FMath::RoundToInt(splineLengthToFill / meshLength))
		                        : 0;

	if (numMeshes == 0)
	{
		return nullptr;
	}

	AActor* splineOwner = spline->GetOwner();
	if (!splineOwner)
	{
		return nullptr;
	}
	
	UInstancedSplineMeshComponent* instancedComponent = CreateInstancedSplineMeshComponent(splineOwner, spline, mesh, makeMovable);
	if (!instancedComponent)
	{
		return nullptr;
	}

	const float segmentLength = splineLengthToFill / numMeshes;
	
	for (int32 i = 0; i < numMeshes; ++i)
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

		instancedComponent->AddSplineInstance(startPos, startTangent, endPos, endTangent, true, true, false);
	}
	
	instancedComponent->MarkRenderStateDirty();
	
	return instancedComponent;
}

void ASSP_Subsystem::HandlePathSplines(AFGDrivingTargetList* targetList, bool show)
{
	if (targetList && !bIsUpdatingPathVisibility)
	{
		bIsUpdatingPathVisibility = true;

		if (show)
		{
			if (UInstancedSplineMeshComponent** existingComponent = VehicleMeshComponents.Find(targetList))
			{
				if (*existingComponent && (*existingComponent)->IsRegistered())
				{
					(*existingComponent)->UnregisterComponent();
					(*existingComponent)->DestroyComponent();
				}
				VehicleMeshComponents.Remove(targetList);
			}

			targetList->SetPathVisible(true);

			auto spline = targetList->GetPath();
			if (spline)
			{
				UInstancedSplineMeshComponent* instancedComponent = BuildSplineMeshes(spline, spline->GetSplineLength(), splinePathMesh, 100);
				if (instancedComponent)
				{
					VehicleMeshComponents.Add(targetList, instancedComponent);
				}
			}
			else
			{
				bool bHasAuthority = GetHasAuthority();
				if (bHasAuthority)
				{
					targetList->CreatePath();
					spline = targetList->GetPath();
					if (spline)
					{
						UInstancedSplineMeshComponent* instancedComponent = BuildSplineMeshes(spline, spline->GetSplineLength(), splinePathMesh, 100);
						if (instancedComponent)
						{
							VehicleMeshComponents.Add(targetList, instancedComponent);
						}
					}
				}
				else
				{
					if (targetList && targetList->GetWorld())
					{
						FTimerHandle RetryTimer;
						targetList->GetWorld()->GetTimerManager().SetTimer(RetryTimer, [this, targetList]()
							{
								auto spline = targetList->GetPath();
								if (spline)
								{
									UInstancedSplineMeshComponent* instancedComponent = BuildSplineMeshes(spline, spline->GetSplineLength(), splinePathMesh, 100);
									if (instancedComponent)
									{
										VehicleMeshComponents.Add(targetList, instancedComponent);
									}
								}
							}, 1.0f, false); // Wait 1 second for replication
					}
				}
			}
		}
		else
		{
			targetList->SetPathVisible(false);

			if (UInstancedSplineMeshComponent** existingComponent = VehicleMeshComponents.Find(targetList))
			{
				if (*existingComponent && (*existingComponent)->IsRegistered())
				{
					(*existingComponent)->UnregisterComponent();
					(*existingComponent)->DestroyComponent();
				}
				VehicleMeshComponents.Remove(targetList);
			}
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
	for (auto& Pair : MantaMeshComponents)
	{
		if (Pair.Value && Pair.Value->IsRegistered())
		{
			Pair.Value->UnregisterComponent();
			Pair.Value->DestroyComponent();
		}
	}
	MantaMeshComponents.Empty();

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
					UInstancedSplineMeshComponent* instancedComponent = BuildSplineMeshes(
						spline, spline->GetSplineLength(), mantaPathMesh, 2500.0f, true);
					
					if (instancedComponent)
					{
						MantaMeshComponents.Add(manta, instancedComponent);
					}
				}
			}
		}
	}
}

void ASSP_Subsystem::HideAllMantaPaths(AActor* actor)
{
	for (auto& Pair : MantaMeshComponents)
	{
		if (Pair.Value && Pair.Value->IsRegistered())
		{
			Pair.Value->UnregisterComponent();
			Pair.Value->DestroyComponent();
		}
	}
	MantaMeshComponents.Empty();
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
