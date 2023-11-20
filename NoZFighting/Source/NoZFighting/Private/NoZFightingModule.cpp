#include "NoZFightingModule.h"
#include "Patching/NativeHookManager.h"
#include "Hologram/FGBuildableHologram.h"
#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildableFoundation.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Buildables/FGBuildableWall.h"
#include "Kismet/KismetMathLibrary.h"
#include "AbstractInstanceManager.h"


//#pragma optimize("", off)
void FNoZFightingModule::StartupModule() {

#if !WITH_EDITOR
	//void AAbstractInstanceManager::SetInstanced(AActor * OwnerActor, const FTransform & ActorTransform, const FInstanceData & InstanceData, FInstanceHandle * &OutHandle, bool bInitializeHidden)
	SUBSCRIBE_METHOD(AAbstractInstanceManager::SetInstanced, [=](auto& scope, AAbstractInstanceManager* self, AActor* OwnerActor, const FTransform& ActorTransform, const FInstanceData& InstanceData, FInstanceHandle*& OutHandle, bool bInitializeHidden)
		{
			SetInstanced(self, OwnerActor, ActorTransform, InstanceData, OutHandle, bInitializeHidden);
		});

	AFGBuildable* bcdo = GetMutableDefault<AFGBuildable>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildable::OnConstruction, bcdo, [=](AFGBuildable* self, const FTransform& transform)
		{
			OnConstruction(self, transform);
		});
#endif
}

void FNoZFightingModule::OnConstruction(AFGBuildable* self, const FTransform& transform)
{
	if (self->GetWorld()->GetName() == "FactoryBlueprintWorld")
	{
		return;
	}
	auto buildFoundation = Cast<AFGBuildableFoundation>(self);
	if (buildFoundation)
	{
		auto ic = self->GetInstigatorController();
		auto config = FNoZFighting_ConfigStruct::GetActiveConfig(buildFoundation->GetWorld());
		auto min = config.FoundationMin;
		auto max = config.FoundationMax;
		auto meshes = self->GetComponentsByClass(UStaticMeshComponent::StaticClass());
		for (auto mesh : meshes)
		{
			auto sMesh = Cast<UStaticMeshComponent>(mesh);
			float randomFloat = UKismetMathLibrary::RandomFloatInRange(min, max);
			sMesh->AddRelativeLocation(FVector(0.0, 0.0, randomFloat));
		}
	}
	else
	{
		auto buildWall = Cast<AFGBuildableWall>(self);
		if (buildWall)
		{
			auto ic = self->GetInstigatorController();
			auto config = FNoZFighting_ConfigStruct::GetActiveConfig(buildWall->GetWorld());
			auto min = config.WallMin;
			auto max = config.WallMax;
			auto meshes = self->GetComponentsByClass(UStaticMeshComponent::StaticClass());
			for (auto mesh : meshes)
			{
				auto sMesh = Cast<UStaticMeshComponent>(mesh);
				float randomFloat = UKismetMathLibrary::RandomFloatInRange(min, max);
				sMesh->AddRelativeLocation(FVector(randomFloat, 0.0, 0.0));
			}
		}
	}
}

void FNoZFightingModule::SetInstanced(AAbstractInstanceManager* manager, AActor* OwnerActor, const FTransform& ActorTransform, const FInstanceData& InstanceData, FInstanceHandle*& OutHandle, bool bInitializeHidden)
{
	FInstanceData* ptr;
	ptr = (FInstanceData*)(&InstanceData);

	auto buildFoundation = Cast<AFGBuildableFoundation>(OwnerActor);
	if (buildFoundation)
	{
		auto config = FNoZFighting_ConfigStruct::GetActiveConfig(OwnerActor->GetWorld());
		auto min = config.FoundationMin;
		auto max = config.FoundationMax;
		float randomFloat = UKismetMathLibrary::RandomFloatInRange(min, max);
		ptr->PositionOffset = FVector(0, 0, randomFloat);
	}
	else if (auto buildWall = Cast<AFGBuildableWall>(OwnerActor))
	{
		auto config = FNoZFighting_ConfigStruct::GetActiveConfig(OwnerActor->GetWorld());
		auto min = config.WallMin;
		auto max = config.WallMax;
		float randomFloat = UKismetMathLibrary::RandomFloatInRange(min, max);
		ptr->PositionOffset = FVector(randomFloat, 0, 0);
		ptr->bApplyRandomOffsetOnInstance = true;
	}
}

//#pragma optimize("", on)

IMPLEMENT_GAME_MODULE(FNoZFightingModule, NoZFighting);