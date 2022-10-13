#include "VPT_Equip.h"
#include "FGSplineMeshGenerationLibrary.h"
#include "FGInstancedSplineMeshComponent.h"
#include "FGSplineMeshResources.h"

DEFINE_LOG_CATEGORY(VPT_Log);
DEFINE_LOG_CATEGORY(LogGame);

AVPT_Equip::AVPT_Equip()
{
}

//void AVPT_Equip::BeginPlay()
//{
//}

void AVPT_Equip::HandleHitActor(AActor* traceHitActor)
{
	this->hitActor = traceHitActor;
}

void AVPT_Equip::PrimaryFirePressed(AFGPlayerController* playerController)
{
	bool leftControlDown = playerController->IsInputKeyDown(EKeys::LeftControl);

	switch (currentMode)
	{
	case EVPT_Mode::VPTM_All_ShowHide:
		!leftControlDown ? ShowAllPaths() : HideAllPaths();
		break;
	case EVPT_Mode::VPTM_SingleNode_ShowHide:
		if (AFGTargetPoint* targetPoint = Cast<AFGTargetPoint>(hitActor))
		{
			!leftControlDown ? ShowTargetPath(targetPoint) : HideTargetPath(targetPoint);
		}
		else if (AFGWheeledVehicle* vehicle = Cast<AFGWheeledVehicle>(hitActor))
		{
			ToggleVehiclePath(vehicle);
		}
		break;
	case EVPT_Mode::VPTM_SingleNode_EditDelete:
		if (AFGTargetPoint* targetPoint = Cast<AFGTargetPoint>(hitActor))
		{
			!leftControlDown ? EditTargetNode(targetPoint) : DeleteTargetNode(targetPoint);
		}
		break;
	case EVPT_Mode::VPTM_SingleNode_CopyPaste:
		if (AFGTargetPoint* targetPoint = Cast<AFGTargetPoint>(hitActor))
		{
			!leftControlDown ? CopyTargetNodeSpeed(targetPoint) : PasteTargetNodeSpeed(targetPoint);
		}
		break;
	case EVPT_Mode::VPTM_SingleVehicle_AI:
		if (AFGWheeledVehicle* vehicle = Cast<AFGWheeledVehicle>(hitActor))
		{
			!leftControlDown ? EnableAIDriver(vehicle) : DisableAIDriver(vehicle);
		}
		break;
	}
}

void AVPT_Equip::SecondaryFirePressed(AFGPlayerController* playerController)
{
	if (!playerController->IsInputKeyDown(EKeys::LeftControl))
	{
		CycleMode();
	}
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
	TArray< USplineMeshComponent* >& globalMeshPool)
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

void AVPT_Equip::CycleMode()
{
	switch (currentMode)
	{
	case EVPT_Mode::VPTM_All_ShowHide:
		currentMode = EVPT_Mode::VPTM_SingleNode_ShowHide;
		break;
	case EVPT_Mode::VPTM_SingleNode_ShowHide:
		currentMode = EVPT_Mode::VPTM_SingleNode_EditDelete;
		break;
	case EVPT_Mode::VPTM_SingleNode_EditDelete:
		currentMode = EVPT_Mode::VPTM_SingleNode_Move;
		break;
	case EVPT_Mode::VPTM_SingleNode_Move:
		currentMode = EVPT_Mode::VPTM_SingleNode_CopyPaste;
		break;
	case EVPT_Mode::VPTM_SingleNode_CopyPaste:
		currentMode = EVPT_Mode::VPTM_SingleVehicle_AI;
		break;
	case EVPT_Mode::VPTM_SingleVehicle_AI:
		currentMode = EVPT_Mode::VPTM_All_ShowHide;
		break;
	}
}

EVPT_Mode AVPT_Equip::GetCurrentMode()
{
	return currentMode;
}



void AVPT_Equip::HandlePathSplines(AFGDrivingTargetList* targetList, bool show)
{
	if (targetList)
	{
		targetList->SetPathVisible(show);
		if (show)
		{
			targetList->CreatePath();
			auto spline = targetList->GetPath();
			if (spline)
			{
				TArray<USplineMeshComponent*> localPool;
				BuildSplineMeshes(spline, spline->GetSplineLength(), splinePathMesh, 100, localPool, pathMeshPool);
				//if (splinePathMaterials.Num() > 0)
				//{
				//	for (auto mesh : localPool)
				//	{
				//		mesh->SetMaterial(0, GetNextMaterial());
				//	}
				//}
			}
		}
	}
}

void AVPT_Equip::ShowAllPaths()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(this->GetWorld(), AFGDrivingTargetList::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		for (auto actor : FoundActors)
		{
			auto tpList = Cast< AFGDrivingTargetList>(actor);
			if (tpList)
			{
				HandlePathSplines(tpList, true);
			}
		}
	}
}

void AVPT_Equip::HideAllPaths()
{
	for (auto meshComp : pathMeshPool)
	{
		if (meshComp && meshComp->IsRegistered())
		{
			meshComp->UnregisterComponent();
		}
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(this->GetWorld(), AFGDrivingTargetList::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		for (auto actor : FoundActors)
		{
			auto tpList = Cast< AFGDrivingTargetList>(actor);
			if (tpList)
			{
				HandlePathSplines(tpList, false);
			}
		}
	}

	pathMeshPool.Empty();
}

void AVPT_Equip::ShowTargetPath(AFGTargetPoint* targetPoint)
{
	auto targetList = targetPoint->GetOwningList();
	if (targetList)
	{
		HandlePathSplines(targetList, true);
	}
}

void AVPT_Equip::HideTargetPath(AFGTargetPoint* targetPoint)
{
	auto targetList = targetPoint->GetOwningList();
	if (targetList)
	{
		HandlePathSplines(targetList, false);
	}
}

void AVPT_Equip::ToggleVehiclePath(AFGWheeledVehicle* vehicle)
{
	auto targetList = vehicle->GetTargetList();
	if (targetList)
	{
		HandlePathSplines(targetList, !targetList->IsPathVisible());
	}
}

void AVPT_Equip::EditTargetNode(AFGTargetPoint* targetPoint)
{
}

void AVPT_Equip::DeleteTargetNode(AFGTargetPoint* targetPoint)
{
	auto targetList = targetPoint->GetOwningList();
	if (targetList)
	{
		targetList->RemoveItem(targetPoint);
		HandlePathSplines(targetList, true);
	}
}

void AVPT_Equip::MoveTargetNode(AFGTargetPoint* targetPoint)
{
	targetPoint->SetActorRelativeLocation(FVector(100, 100, 100));
	auto targetList = targetPoint->GetOwningList();
	if (targetList)
	{
		HandlePathSplines(targetList, true);
	}
}


void AVPT_Equip::CopyTargetNodeSpeed(AFGTargetPoint* targetPoint)
{
}

void AVPT_Equip::CopyTargetNodeWaitTime(AFGTargetPoint* targetPoint)
{
}

void AVPT_Equip::PasteTargetNodeSpeed(AFGTargetPoint* targetPoint)
{
}

void AVPT_Equip::PasteTargetNodeWaitTime(AFGTargetPoint* targetPoint)
{
}

void AVPT_Equip::EnableAIDriver(AFGWheeledVehicle* vehicle)
{
	vehicle->Server_ToggleAutoPilot();
}

void AVPT_Equip::DisableAIDriver(AFGWheeledVehicle* vehicle)
{
	vehicle->Server_ToggleAutoPilot();
}

UMaterialInterface* AVPT_Equip::GetNextMaterial()
{
	if (splinePathMaterials.Num() > 0)
	{
		UMaterialInterface* currentMaterial = splinePathMaterials[currentMaterialIndex];
		if (currentMaterialIndex == splinePathMaterials.Num() - 1)
		{
			currentMaterialIndex = 0;
		}
		else
		{
			currentMaterialIndex++;
		}
		return currentMaterial;
	}
	return nullptr;
}
