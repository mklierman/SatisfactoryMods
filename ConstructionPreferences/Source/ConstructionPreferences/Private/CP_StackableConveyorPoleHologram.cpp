#include "CP_StackableConveyorPoleHologram.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "FGFactoryConnectionComponent.h"
#include "UObject/ConstructorHelpers.h"

static constexpr float StackableBeltSnapHeight = 300.0f;

ACP_StackableConveyorPoleHologram::ACP_StackableConveyorPoleHologram()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> stackableMeshFinder(
		TEXT("/Game/FactoryGame/Buildable/Factory/ConveyorPoleStackable/Mesh/ConveyorPoleMulti_01_static.ConveyorPoleMulti_01_static"));
	mStackablePoleMesh = stackableMeshFinder.Object;
}

void ACP_StackableConveyorPoleHologram::BeginPlay()
{
	const TSubclassOf<AActor> stackableBuildClass = GetActorClass();
	const TSubclassOf<AActor> compatibilityPoleClass = LoadClass<AActor>(
		nullptr,
		TEXT("/Game/FactoryGame/Buildable/Factory/ConveyorPole/Build_ConveyorPole.Build_ConveyorPole_C"));
	if (IsValid(compatibilityPoleClass))
	{
		SetBuildClass(compatibilityPoleClass);
	}

	AFGConveyorPoleHologram::BeginPlay();
	SetBuildClass(stackableBuildClass);
	UpdatePoleMesh();
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
