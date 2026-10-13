#include "CP_StackablePipelinePoleHologram.h"

#include "Buildables/FGBuildablePoleStackable.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "FGPipeConnectionComponent.h"
#include "FGPlayerController.h"
#include "UObject/ConstructorHelpers.h"

static constexpr float StackableSupportSnapHeight = 375.0f;

ACP_StackablePipelinePoleHologram::ACP_StackablePipelinePoleHologram()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> pipeMeshFinder(TEXT("/Game/FactoryGame/Buildable/Factory/PipePole/Mesh/SM_PipePoleMulti_01.SM_PipePoleMulti_01"));
	mPipeStackableMesh = pipeMeshFinder.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> hypertubeMeshFinder(TEXT("/Game/FactoryGame/Buildable/Factory/PipeHyperSupport/Mesh/SM_HyperTubePoleMulti_01.SM_HyperTubePoleMulti_01"));
	mHypertubeStackableMesh = hypertubeMeshFinder.Object;
}

void ACP_StackablePipelinePoleHologram::BeginPlay()
{
	const TSubclassOf<AActor> stackableBuildClass = GetActorClass();
	const bool bIsHypertube = IsValid(stackableBuildClass) && stackableBuildClass->GetName().Contains(TEXT("Hyper"), ESearchCase::IgnoreCase);
	mActiveStackableMesh = bIsHypertube ? mHypertubeStackableMesh : mPipeStackableMesh;

	const TSubclassOf<AActor> compatibilitySupportClass = LoadClass<AActor>(nullptr, bIsHypertube ? TEXT("/Game/FactoryGame/Buildable/Factory/PipeHyperSupport/Build_PipeHyperSupport.Build_PipeHyperSupport_C") : TEXT("/Game/FactoryGame/Buildable/Factory/PipelineSupport/Build_PipelineSupport.Build_PipelineSupport_C"));
	if (IsValid(compatibilitySupportClass))
	{
		SetBuildClass(compatibilitySupportClass);
	}

	AFGPipelinePoleHologram::BeginPlay();
	SetBuildClass(stackableBuildClass);
	UpdatePoleMesh();
}

bool ACP_StackablePipelinePoleHologram::DoMultiStepPlacement(bool isInputFromARelease)
{
	return AFGBuildableHologram::DoMultiStepPlacement(isInputFromARelease);
}

void ACP_StackablePipelinePoleHologram::SetHologramLocationAndRotation(const FHitResult& hitResult)
{
	if (!TrySnapToStackablePole(hitResult))
	{
		Super::SetHologramLocationAndRotation(hitResult);
	}
}

bool ACP_StackablePipelinePoleHologram::TrySnapToStackablePole(const FHitResult& hitResult)
{
	APawn* instigator = GetConstructionInstigator();
	AFGPlayerController* controller = IsValid(instigator) ? Cast<AFGPlayerController>(instigator->GetController()) : nullptr;
	if (!IsValid(controller) && IsValid(GetWorld()))
	{
		controller = Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController());
	}

	AFGBuildablePoleStackable* stackablePole = Cast<AFGBuildablePoleStackable>(hitResult.GetActor());
	if (IsValid(controller) &&
		controller->IsLocalController() &&
		(controller->IsInputKeyDown(EKeys::LeftControl) || controller->IsInputKeyDown(EKeys::RightControl)) &&
		IsValid(stackablePole))
	{
		const bool targetChanged = mStackedOnPole != stackablePole;
		if (targetChanged)
		{
			mStackRotationOffset = 0.0f;
		}

		const FVector location = stackablePole->GetActorLocation() + stackablePole->GetActorUpVector() * stackablePole->GetStackHeight();
		FRotator rotation = stackablePole->GetActorRotation();
		rotation.Yaw += mStackRotationOffset;
		SetActorLocationAndRotation(location, rotation);
		mSnappedBuilding = stackablePole;
		mStackedOnPole = stackablePole;
		if (targetChanged)
		{
			OnSnap();
		}
		return true;
	}

	mSnappedBuilding = nullptr;
	mStackedOnPole = nullptr;
	mStackRotationOffset = 0.0f;
	return false;
}

bool ACP_StackablePipelinePoleHologram::TrySnapToActor(const FHitResult& hitResult)
{
	return TrySnapToStackablePole(hitResult) || Super::TrySnapToActor(hitResult);
}

void ACP_StackablePipelinePoleHologram::RotateStackedPole(int32 delta)
{
	if (!IsValid(mStackedOnPole))
	{
		return;
	}

	mStackRotationOffset = FRotator::NormalizeAxis(mStackRotationOffset + delta * 180.0f);
	FRotator rotation = mStackedOnPole->GetActorRotation();
	rotation.Yaw += mStackRotationOffset;
	SetActorRotation(rotation);
}

void ACP_StackablePipelinePoleHologram::CheckValidFloor()
{
	if (!IsValid(mStackedOnPole))
	{
		Super::CheckValidFloor();
	}
}

bool ACP_StackablePipelinePoleHologram::IsHologramIdenticalToActor(AActor* actor, const FTransform& hologramTransform) const
{
	if (const AFGBuildablePoleStackable* pole = Cast<AFGBuildablePoleStackable>(actor);
		IsValid(pole) && pole->GetActorLocation().Equals(hologramTransform.GetLocation(), 10.0f))
	{
		return true;
	}

	return Super::IsHologramIdenticalToActor(actor, hologramTransform);
}

void ACP_StackablePipelinePoleHologram::UpdatePoleMesh()
{
	if (UStaticMeshComponent* poleMesh = GetPoleMesh(); IsValid(poleMesh) && IsValid(mActiveStackableMesh))
	{
		poleMesh->SetStaticMesh(mActiveStackableMesh);
		poleMesh->SetRelativeScale3D(FVector::OneVector);
		poleMesh->SetVisibility(true, true);
	}

	if (IsValid(mSupportTopPartMeshComponent))
	{
		mSupportTopPartMeshComponent->SetVisibility(false, true);
		mSupportTopPartMeshComponent->SetHiddenInGame(true, true);
	}

	if (UFGPipeConnectionComponentBase* snapConnection = GetSnapConnection(); IsValid(snapConnection))
	{
		FVector connectionLocation = snapConnection->GetRelativeLocation();
		connectionLocation.Z = StackableSupportSnapHeight;
		snapConnection->SetRelativeLocationAndRotation(connectionLocation, FRotator::ZeroRotator);
	}
}

void ACP_StackablePipelinePoleHologram::ConfigureActor(AFGBuildable* inBuildable) const
{
	AFGBuildableHologram::ConfigureActor(inBuildable);
}
