// ILikeBanas

#include "CDO_BPLib.h"
#include "CDOModule.h"
#include "AI/FGAISystem.h"
#include "FGRailroadVehicleMovementComponent.h"
#include "Equipment/FGResourceScanner.h"
#include "Blueprint/UserWidget.h"
#include "Patching/BlueprintHookHelper.h"
#include "Patching/BlueprintHookManager.h"
#include "WheeledVehicles/FGTargetPointLinkedList.h"
#include "Hologram/FGConveyorLiftHologram.h"
#include "FGSplineMeshGenerationLibrary.h"
#include "FGInstancedSplineMeshComponent.h"
#include "FGSplineMeshResources.h"
#include "Components/SplineMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "SplineCustomVersion.h"
#include "FGManta.h"


//#include "ProxyInstancedStaticMeshComponent.h"
#include "FGColoredInstanceMeshProxy.h"
#include "FGOutlineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "FGProductionIndicatorInstanceComponent.h"

#include "Components/StaticMeshComponent.h"
#include <Runtime/Engine/Classes/Components/ProxyInstancedStaticMeshComponent.h>

//void UCDO_BPLib::BindOnWidgetConstruct(const TSubclassOf<UUserWidget> WidgetClass, FOnWidgetCreated Binding) {
//	if (!WidgetClass)
//		return;
//	UFunction* ConstructFunction = WidgetClass->FindFunctionByName(TEXT("Construct"));
//	if (!ConstructFunction || ConstructFunction->IsNative())
//	{
//		return;
//	}
//	UBlueprintHookManager* HookManager = GEngine->GetEngineSubsystem<UBlueprintHookManager>();
//	HookManager->HookBlueprintFunction(ConstructFunction, [Binding](FBlueprintHookHelper& HookHelper) {
//		Binding.ExecuteIfBound(Cast<UUserWidget>(HookHelper.GetContext()));
//		}, EPredefinedHookOffset::Return);
//}
void SetMeshInstanced(UMeshComponent* MeshComp, bool Instanced)
{
	auto StaticMeshProxy = Cast<UProxyInstancedStaticMeshComponent>(MeshComp);
	if (StaticMeshProxy) {
		StaticMeshProxy->SetInstanced(Instanced);
	}
	else {
		auto ColoredMeshProxy = Cast<UFGColoredInstanceMeshProxy>(MeshComp);
		if (ColoredMeshProxy) {
			ColoredMeshProxy->mBlockInstancing = !Instanced;
			ColoredMeshProxy->SetInstanced(Instanced);
		}
		else {
			auto ProdIndInst = Cast<UFGProductionIndicatorInstanceComponent>(MeshComp);
			if (ProdIndInst) {
				ProdIndInst->SetInstanced(Instanced);
			}
		}
	}
}

void RefreshMaterial(AActor* Actor)
{
	// hack to refresh static mesh - This is needed for power indicators and meshes to update properly.
	auto meshes = Actor->GetComponentsByClass(UMeshComponent::StaticClass());
	for (auto mesh : meshes)
	{
		auto meshComp = Cast<UMeshComponent>(mesh);
		if (meshComp)
		{
			SetMeshInstanced(meshComp, true);
		}
	}
	//auto OutlineComp = UFGOutlineComponent::Get(Actor->GetWorld());
	//OutlineComp->HideAllDismantlePendingMaterial();
	//OutlineComp->ShowDismantlePendingMaterial(Actor);
	// reset color slot for this actor (e.g. for buggy ramps)
	//auto Buildable = Cast<AFGBuildable>(Actor);
	//if (Buildable) {
	//	Buildable->SetColorSlot_Implementation(Buildable->GetColorSlot_Implementation());
	//}

}


void UCDO_BPLib::SetLiftProperties(float stepHeight, float minHeight)
{
	AFGConveyorLiftHologram* hg = GetMutableDefault<AFGConveyorLiftHologram>();
	hg->mStepHeight = stepHeight;
	hg->mMinimumHeight = minHeight;
}

void UCDO_BPLib::MoveBuildable(AFGBuildable* inBuildable, FVector newLocation)
{
	auto scene = inBuildable->GetRootComponent();
	auto SavedMobility = scene->Mobility;
	scene->SetMobility(EComponentMobility::Movable);
	scene->SetWorldLocation(newLocation, false, nullptr, ETeleportType::TeleportPhysics);
	if (SavedMobility != EComponentMobility::Movable) {
		scene->SetMobility(EComponentMobility::Static);
	}
	RefreshMaterial(inBuildable);
}

void UCDO_BPLib::ShowVehiclePaths(UObject* WorldContextObj, UStaticMesh* mesh)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObj->GetWorld(), AFGDrivingTargetList::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		for (auto actor : FoundActors)
		{
			auto tpList = Cast< AFGDrivingTargetList>(actor);
			if (tpList)
			{
				TArray< USplineMeshComponent* > meshPool;
				meshPool.Empty();
				tpList->CreatePath();
				auto spline = tpList->GetPath();
				if (spline)
				{
					UFGSplineMeshGenerationLibrary::BuildSplineMeshes(spline, spline->GetSplineLength(), mesh, 100, meshPool, [](USplineComponent* spline)
						{
							USplineMeshComponent* newComponent = NewObject< USplineMeshComponent >(spline->GetOwner(), USplineMeshComponent::StaticClass());
							newComponent->SetupAttachment(spline);
							newComponent->Mobility = EComponentMobility::Movable;
							return newComponent;
						});
				}
			}
		}
	}

	//TArray<AActor*> FoundMActors;
	//UGameplayStatics::GetAllActorsOfClass(WorldContextObj->GetWorld(), AFGManta::StaticClass(), FoundMActors);
	//if (FoundActors.Num() > 0)
	//{
	//	for (auto actor : FoundMActors)
	//	{
	//		auto manta = Cast< AFGManta>(actor);
	//		if (manta)
	//		{
	//			auto spline = manta->GetSpline();
	//			UFGSplineMeshGenerationLibrary::BuildSplineMeshes(spline, spline->GetSplineLength(), mesh, 400, meshPool, [](USplineComponent* spline)
	//				{
	//					USplineMeshComponent* newComponent = NewObject< USplineMeshComponent >(spline->GetOwner(), USplineMeshComponent::StaticClass());

	//					newComponent->SetupAttachment(spline);
	//					newComponent->Mobility = EComponentMobility::Movable;

	//					// Custom initialization steps

	//					return newComponent;
	//				});
	//		}
	//	}
	//}
}

TArray<USplineComponent*> UCDO_BPLib::GetAllVehiclePathSplines(UObject* WorldContextObj)
{
	TArray<USplineComponent*> vehicleSplines;
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObj->GetWorld(), AFGDrivingTargetList::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		for (auto actor : FoundActors)
		{
			auto tpList = Cast< AFGDrivingTargetList>(actor);
			if (tpList)
			{
				vehicleSplines.Add(tpList->GetPath());
			}
		}
	}
	return vehicleSplines;
}

