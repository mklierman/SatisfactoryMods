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
	auto buildFoundation = Cast<AFGBuildableFoundation>(self);
	if (buildFoundation)
	{
		auto meshes = self->GetComponentsByClass(UStaticMeshComponent::StaticClass());
		for (auto mesh : meshes)
		{
			auto sMesh = Cast<UStaticMeshComponent>(mesh);
			float randomFloat = UKismetMathLibrary::RandomFloatInRange(-1.25, 1.25);
			sMesh->AddRelativeLocation(FVector(0.0, 0.0, randomFloat));
		}
	}
	else
	{
		auto buildWall = Cast<AFGBuildableWall>(self);
		if (buildWall)
		{
			auto meshes = self->GetComponentsByClass(UStaticMeshComponent::StaticClass());
			for (auto mesh : meshes)
			{
				auto sMesh = Cast<UStaticMeshComponent>(mesh);
				float randomFloat = UKismetMathLibrary::RandomFloatInRange(-0.35, 0.35);
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
		float randomFloat = UKismetMathLibrary::RandomFloatInRange(-1.05, 1.05);
		ptr->PositionOffset = FVector(0, 0, randomFloat);
	}
	else if (auto buildWall = Cast<AFGBuildableWall>(OwnerActor))
	{
		float randomFloat = UKismetMathLibrary::RandomFloatInRange(-0.15, 0.15);
		ptr->PositionOffset = FVector(randomFloat, 0, 0);
		ptr->bApplyRandomOffsetOnInstance = true;
	}
}

//#pragma optimize("", on)

IMPLEMENT_GAME_MODULE(FNoZFightingModule, NoZFighting);