#include "SSP_Subsystem.h"
#include "FGSplineMeshGenerationLibrary.h"

bool ASSP_Subsystem::GetHasAuthority()
{
	return HasAuthority();
}

//#pragma optimize("", off)
bool ASSP_Subsystem::ShouldSave_Implementation() const
{
    return true;
}

USplineMeshComponent* SplineMeshConstructor(USplineComponent* spline)
{
	USplineMeshComponent* newComponent = NewObject< USplineMeshComponent >(spline->GetOwner(), USplineMeshComponent::StaticClass());
	newComponent->SetupAttachment(spline);
	newComponent->Mobility = EComponentMobility::Static;
	return newComponent;
}

void BuildSplineMeshes(
	class USplineComponent* spline,
	float maxSplineLengthToFill,
	UStaticMesh* mesh,
	float meshLength,
	TArray< USplineMeshComponent* >& localMeshPool,
	TArray< USplineMeshComponent* >& globalMeshPool,
	bool makeMovable = false)
{
	const float splineLengthToFill = FMath::Clamp(spline->GetSplineLength(), 0.f, maxSplineLengthToFill);
	const int32 numMeshes = splineLengthToFill > SMALL_NUMBER ? FMath::Max(1, FMath::RoundToInt(splineLengthToFill / meshLength)) : 0;

	// Create more or remove the excess meshes.
	if (numMeshes < localMeshPool.Num())
	{
		while (localMeshPool.Num() > numMeshes)
		{
			localMeshPool.Last()->DestroyComponent();
			localMeshPool.Pop();
		}
	}
	else if (numMeshes > localMeshPool.Num())
	{
		while (localMeshPool.Num() < numMeshes)
		{
			if (auto newMesh = SplineMeshConstructor(spline))
			{
				if (makeMovable)
				{
					newMesh->SetMobility(EComponentMobility::Movable);
				}
				localMeshPool.Push(newMesh);
				globalMeshPool.Add(newMesh);
			}
			else
			{
				break;
			}
		}
	}

	// Put all pieces along the spline.
	{
		const float segmentLength = splineLengthToFill / numMeshes;

		for (int32 i = 0; i < localMeshPool.Num(); ++i)
		{
			const float startDistance = (float)i * segmentLength;
			const float endDistance = (float)(i + 1) * segmentLength;
			const FVector startPos = spline->GetLocationAtDistanceAlongSpline(startDistance, ESplineCoordinateSpace::Local);
			const FVector startTangent = spline->GetTangentAtDistanceAlongSpline(startDistance, ESplineCoordinateSpace::Local).GetSafeNormal() * segmentLength;
			const FVector endPos = spline->GetLocationAtDistanceAlongSpline(endDistance, ESplineCoordinateSpace::Local);
			const FVector endTangent = spline->GetTangentAtDistanceAlongSpline(endDistance, ESplineCoordinateSpace::Local).GetSafeNormal() * segmentLength;

			localMeshPool[i]->SetStartAndEnd(startPos, startTangent, endPos, endTangent, true);
			localMeshPool[i]->SetStaticMesh(mesh);
		}
	}

	// Register new meshes, needs to happen after the properties are set for static components.
	for (auto meshComp : localMeshPool)
	{
		if (!meshComp->IsRegistered())
		{
			meshComp->RegisterComponent();
		}
	}
}



void ASSP_Subsystem::HandlePathSplines(AFGDrivingTargetList* targetList, bool show)
{
	if (HasAuthority() && targetList)
	{
		//targetList->SetPathVisible(show);
		if (show)
		{
			targetList->CalculateTargetCount();
			targetList->CreatePath();
			auto spline = targetList->GetPath();
			if (spline)
			{
				TArray<USplineMeshComponent*> localPool;
				BuildSplineMeshes(spline, spline->GetSplineLength(), splinePathMesh, 100, localPool, pathMeshPool);

				FSSP_MeshPoolStruct pool;
				pool.MeshPool = localPool;
				TargetListMeshPools.Add(targetList, pool);
			}
		}
		else
		{
			if (TargetListMeshPools.Contains(targetList))
			{
				TArray<USplineMeshComponent*> meshPool = TargetListMeshPools[targetList].MeshPool;
				for (auto meshComp : meshPool)
				{
					if (meshComp && meshComp->IsRegistered())
					{
						meshComp->UnregisterComponent();
					}
				}
				meshPool.Empty();
				TargetListMeshPools.Remove(targetList);
			}
		}
	}
}

void ASSP_Subsystem::ShowTargetPath(AFGTargetPoint* targetPoint)
{
	auto targetList = targetPoint->GetOwningList();
	if (HasAuthority() && targetList)
	{
		targetList->SetPathVisible(true);
		//HandlePathSplines(targetList, true);
	}
}

void ASSP_Subsystem::HideTargetPath(AFGTargetPoint* targetPoint)
{
	auto targetList = targetPoint->GetOwningList();
	if (HasAuthority() && targetList)
	{
		targetList->SetPathVisible(false);
		//HandlePathSplines(targetList, false);
	}
}

void ASSP_Subsystem::ToggleVehiclePath(AFGWheeledVehicle* vehicle)
{
	auto targetList = vehicle->GetTargetList();
	if (HasAuthority() && targetList)
	{
		HandlePathSplines(targetList, !targetList->IsPathVisible());
	}
}
void ASSP_Subsystem::ShowInitialPaths(AActor* actor)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(actor->GetWorld(), AFGDrivingTargetList::StaticClass(), FoundActors);
	if (HasAuthority() && FoundActors.Num() > 0)
	{
		for (AActor* actor : FoundActors)
		{
			auto tpList = Cast< AFGDrivingTargetList>(actor);
			if (tpList && tpList->mIsPathVisible)
			{
				HandlePathSplines(tpList, true);
			}
		}
	}
}
void ASSP_Subsystem::ShowAllMantaPaths(AActor* actor)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(actor->GetWorld(), AFGManta::StaticClass(), FoundActors);
	if (HasAuthority() && FoundActors.Num() > 0)
	{
		for (auto actor : FoundActors)
		{
			auto manta = Cast< AFGManta>(actor);
			if (manta)
			{
				auto spline = manta->mCachedSpline;
				if (spline)
				{
					TArray<USplineMeshComponent*> localPool;
					BuildSplineMeshes(spline, spline->GetSplineLength(), mantaPathMesh, 2500, localPool, pathMeshPool, true);

					FSSP_MeshPoolStruct pool;
					pool.MeshPool = localPool;
					MantaMeshPools.Add(manta, pool);
				}
			}
		}
	}
}
void ASSP_Subsystem::HideAllMantaPaths(AActor* actor)
{
	if (HasAuthority())
	{
		for (auto mantaPair : MantaMeshPools)
		{

			TArray<USplineMeshComponent*> meshPool = mantaPair.Value.MeshPool;
			for (auto meshComp : meshPool)
			{
				if (meshComp && meshComp->IsRegistered())
				{
					meshComp->UnregisterComponent();
				}
			}
			meshPool.Empty();
		}
		MantaMeshPools.Empty();
	}
}
void ASSP_Subsystem::ShowAllVehiclePaths(AActor* actor)
{
	if (HasAuthority())
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(actor->GetWorld(), AFGDrivingTargetList::StaticClass(), FoundActors);
		if (FoundActors.Num() > 0)
		{
			for (auto actor : FoundActors)
			{
				auto tpList = Cast< AFGDrivingTargetList>(actor);
				tpList->SetPathVisible(true);
			}
		}
	}
}
void ASSP_Subsystem::HideAllVehiclePaths(AActor* actor)
{
	if (HasAuthority())
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(actor->GetWorld(), AFGDrivingTargetList::StaticClass(), FoundActors);
		if (FoundActors.Num() > 0)
		{
			for (auto actor : FoundActors)
			{
				auto tpList = Cast< AFGDrivingTargetList>(actor);
				tpList->SetPathVisible(false);
			}
		}
	}
}
//#pragma optimize("", on)