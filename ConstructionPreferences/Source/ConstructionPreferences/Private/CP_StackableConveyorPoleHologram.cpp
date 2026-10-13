#include "CP_StackableConveyorPoleHologram.h"

#include "Buildables/FGBuildablePoleStackable.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "FGFactoryConnectionComponent.h"
#include "FGPlayerController.h"
#include "UObject/ConstructorHelpers.h"

static constexpr float StackableBeltSnapHeight = 300.0f;

ACP_StackableConveyorPoleHologram::ACP_StackableConveyorPoleHologram()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> stackableMeshFinder(TEXT("/Game/FactoryGame/Buildable/Factory/ConveyorPoleStackable/Mesh/ConveyorPoleMulti_01_static.ConveyorPoleMulti_01_static"));
	mStackablePoleMesh = stackableMeshFinder.Object;
}

void ACP_StackableConveyorPoleHologram::BeginPlay()
{
	const TSubclassOf<AActor> stackableBuildClass = GetActorClass();
	const TSubclassOf<AActor> compatibilityPoleClass = LoadClass<AActor>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/ConveyorPole/Build_ConveyorPole.Build_ConveyorPole_C"));
	if (IsValid(compatibilityPoleClass))
	{
		SetBuildClass(compatibilityPoleClass);
	}

	AFGConveyorPoleHologram::BeginPlay();
	SetBuildClass(stackableBuildClass);
	UpdatePoleMesh();
}

bool ACP_StackableConveyorPoleHologram::DoMultiStepPlacement(bool isInputFromARelease)
{
	return AFGBuildableHologram::DoMultiStepPlacement(isInputFromARelease);
}

void ACP_StackableConveyorPoleHologram::SetHologramLocationAndRotation(const FHitResult& hitResult)
{
	if (!TrySnapToStackablePole(hitResult))
	{
		Super::SetHologramLocationAndRotation(hitResult);
	}
}

bool ACP_StackableConveyorPoleHologram::TrySnapToStackablePole(const FHitResult& hitResult)
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
		SetSnappBuilding(stackablePole);
		mStackedOnPole = stackablePole;
		if (targetChanged)
		{
			OnSnap();
		}
		return true;
	}

	mStackedOnPole = nullptr;
	mStackRotationOffset = 0.0f;
	return false;
}

bool ACP_StackableConveyorPoleHologram::TrySnapToActor(const FHitResult& hitResult)
{
	return TrySnapToStackablePole(hitResult) || Super::TrySnapToActor(hitResult);
}

void ACP_StackableConveyorPoleHologram::RotateStackedPole(int32 delta)
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

void ACP_StackableConveyorPoleHologram::CheckValidFloor()
{
	if (!IsValid(mStackedOnPole))
	{
		Super::CheckValidFloor();
	}
}

bool ACP_StackableConveyorPoleHologram::IsHologramIdenticalToActor(AActor* actor, const FTransform& hologramTransform) const
{
	if (const AFGBuildablePoleStackable* pole = Cast<AFGBuildablePoleStackable>(actor);
		IsValid(pole) && pole->GetActorLocation().Equals(hologramTransform.GetLocation(), 10.0f))
	{
		return true;
	}

	return Super::IsHologramIdenticalToActor(actor, hologramTransform);
}

void ACP_StackableConveyorPoleHologram::UpdatePoleMesh()
{
	if (UStaticMeshComponent* poleMesh = GetPoleMesh(); IsValid(poleMesh) && IsValid(mStackablePoleMesh))
	{
		poleMesh->SetStaticMesh(mStackablePoleMesh);
		poleMesh->SetRelativeScale3D(FVector::OneVector);
		poleMesh->SetVisibility(true, true);
	}

	if (UFGFactoryConnectionComponent* snapConnection = GetSnapConnection(); IsValid(snapConnection))
	{
		FVector connectionLocation = snapConnection->GetRelativeLocation();
		connectionLocation.Z = StackableBeltSnapHeight;
		snapConnection->SetRelativeLocationAndRotation(connectionLocation, FRotator::ZeroRotator);
	}
}

void ACP_StackableConveyorPoleHologram::ConfigureActor(AFGBuildable* inBuildable) const
{
	AFGBuildableHologram::ConfigureActor(inBuildable);
}
