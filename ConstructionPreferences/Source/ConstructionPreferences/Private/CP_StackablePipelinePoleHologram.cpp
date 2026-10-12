#include "CP_StackablePipelinePoleHologram.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "FGPipeConnectionComponent.h"
#include "UObject/ConstructorHelpers.h"

static constexpr float StackableSupportSnapHeight = 375.0f;

ACP_StackablePipelinePoleHologram::ACP_StackablePipelinePoleHologram()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> pipeMeshFinder(
		TEXT("/Game/FactoryGame/Buildable/Factory/PipePole/Mesh/SM_PipePoleMulti_01.SM_PipePoleMulti_01"));
	mPipeStackableMesh = pipeMeshFinder.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> hypertubeMeshFinder(
		TEXT("/Game/FactoryGame/Buildable/Factory/PipeHyperSupport/Mesh/SM_HyperTubePoleMulti_01.SM_HyperTubePoleMulti_01"));
	mHypertubeStackableMesh = hypertubeMeshFinder.Object;
}

void ACP_StackablePipelinePoleHologram::BeginPlay()
{
	const TSubclassOf<AActor> stackableBuildClass = GetActorClass();
	const bool bIsHypertube = IsValid(stackableBuildClass) &&
		stackableBuildClass->GetName().Contains(TEXT("Hyper"), ESearchCase::IgnoreCase);
	mActiveStackableMesh = bIsHypertube ? mHypertubeStackableMesh : mPipeStackableMesh;

	const TSubclassOf<AActor> compatibilitySupportClass = LoadClass<AActor>(
		nullptr,
		bIsHypertube
			? TEXT("/Game/FactoryGame/Buildable/Factory/PipeHyperSupport/Build_PipeHyperSupport.Build_PipeHyperSupport_C")
			: TEXT("/Game/FactoryGame/Buildable/Factory/PipelineSupport/Build_PipelineSupport.Build_PipelineSupport_C"));
	if (IsValid(compatibilitySupportClass))
	{
		SetBuildClass(compatibilitySupportClass);
	}

	AFGPipelinePoleHologram::BeginPlay();
	SetBuildClass(stackableBuildClass);
	UpdatePoleMesh();
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
