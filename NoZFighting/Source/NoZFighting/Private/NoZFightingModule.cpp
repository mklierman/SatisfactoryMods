#include "NoZFightingModule.h"
#include "Patching/NativeHookManager.h"
#include "Hologram/FGBuildableHologram.h"
#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildableFoundation.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Buildables/FGBuildableWall.h"
#include "Kismet/KismetMathLibrary.h"
#include "FGLightweightBuildableSubsystem.h"
#include <Buildables/FGBuildableWalkway.h>
#include <Hologram/FGFoundationHologram.h>


#pragma optimize("", off)
void FNoZFightingModule::StartupModule() {
#if !WITH_EDITOR
	SUBSCRIBE_METHOD(AAbstractInstanceManager::SetInstanced, [this](auto& scope, AAbstractInstanceManager* self, AActor* OwnerActor, const FTransform& ActorTransform, const FInstanceData& InstanceData, FInstanceOwnerHandlePtr& OutHandle, bool bInitializeHidden)
		{
			SetInstanced(self, OwnerActor, ActorTransform, InstanceData, OutHandle, bInitializeHidden);
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
	auto buildWall = Cast<AFGBuildableWall>(self);
	auto buildWalkway = Cast<AFGBuildableWalkway>(self);
	if (buildFoundation || buildWalkway || self)
	{
		auto ic = self->GetInstigatorController();
		UWorld* world = self->GetWorld();
		auto config = FNoZFighting_ConfigStruct::GetActiveConfig(world);
		auto min = config.FoundationMin;
		auto max = config.FoundationMax;
		auto meshes = self->GetComponents();
		for (auto mesh : meshes)
		{
			auto sMesh = Cast<UStaticMeshComponent>(mesh);
			if (sMesh)
			{
				float randomFloat = UKismetMathLibrary::RandomFloatInRange(min, max);
				sMesh->AddRelativeLocation(FVector(0.0, 0.0, randomFloat));
			}
			auto cMesh = Cast<UFGColoredInstanceMeshProxy>(mesh);
			if (cMesh)
			{
				float randomFloat = UKismetMathLibrary::RandomFloatInRange(min, max);
				cMesh->AddRelativeLocation(FVector(0.0, 0.0, randomFloat));
			}
		}
	}
	else if (buildWall)
	{
		auto ic = self->GetInstigatorController();
		auto config = FNoZFighting_ConfigStruct::GetActiveConfig(buildWall->GetWorld());
		auto min = config.WallMin;
		auto max = config.WallMax;
		auto meshes = self->GetComponents();
		for (auto mesh : meshes)
		{
			auto sMesh = Cast<UStaticMeshComponent>(mesh);
			if (sMesh)
			{
				float randomFloat = UKismetMathLibrary::RandomFloatInRange(min, max);
				sMesh->AddRelativeLocation(FVector(randomFloat, 0.0, 0.0));
			}
		}
	}
}

void FNoZFightingModule::SetInstanced(AAbstractInstanceManager* manager, AActor* OwnerActor, const FTransform& ActorTransform, const FInstanceData& InstanceData, FInstanceOwnerHandlePtr& OutHandle, bool bInitializeHidden)
{

	FName Name;
	if (InstanceData.OverridenMaterials.Num() > 0)
	{
		uint32 Hash = 0;
		for (const UMaterialInterface* Material : InstanceData.OverridenMaterials)
		{
			if (Material)
			{
				Hash = HashCombine(Hash, GetTypeHash(Material->GetFName()));
			}
		}

		Name = FName(InstanceData.StaticMesh->GetName(), Hash);
	}
	else
	{
		Name = InstanceData.StaticMesh->GetFName();
	}

	FInstanceComponentData* InstanceEntry = manager->InstanceMap.Find(Name);
	FInstanceData* ptr;
	ptr = (FInstanceData*)(&InstanceData);

	if (Name.ToString().ToLower().Contains("foundation") || Name.ToString().ToLower().Contains("block")
		|| Name.ToString().ToLower().Contains("catwalk") || Name.ToString().ToLower().Contains("walkway")
		|| Name.ToString().ToLower().Contains("quarterpipe") || Name.ToString().ToLower().Contains("ramp"))
	{
		auto config = FNoZFighting_ConfigStruct::GetActiveConfig(OwnerActor->GetWorld());
		auto min = config.FoundationMin;
		auto max = config.FoundationMax;
		float randomFloat = UKismetMathLibrary::RandomFloatInRange(min, max);
		auto loc = ptr->RelativeTransform.GetLocation();
		loc.Z = loc.Z + randomFloat;
		ptr->RelativeTransform.SetLocation(loc);
	}
	else if (Name.ToString().ToLower().Contains("wall"))
	{
		ptr->bApplyRandomOffsetOnInstance = false;

		auto config = FNoZFighting_ConfigStruct::GetActiveConfig(OwnerActor->GetWorld());
		auto min = config.WallMin;
		auto max = config.WallMax;
		float randomFloat = UKismetMathLibrary::RandomFloatInRange(min, max);
		auto loc = ptr->RelativeTransform.GetLocation();
		loc.X = loc.X + randomFloat;
		ptr->RelativeTransform.SetLocation(loc);
	}
	else if (Name.ToString().ToLower().Contains("beam") || Name.ToString().ToLower().Contains("pillar")
		|| Name.ToString().ToLower().Contains("smallMiddle"))
	{
		ptr->bApplyRandomOffsetOnInstance = false;

		auto config = FNoZFighting_ConfigStruct::GetActiveConfig(OwnerActor->GetWorld());
		auto min = config.BeamMin;
		auto max = config.BeamMax;
		float randomFloat = UKismetMathLibrary::RandomFloatInRange(min, max);
		auto loc = ptr->RelativeTransform.GetLocation();
		loc.X = loc.X + randomFloat;
		loc.Y = loc.Y + randomFloat;
		loc.Z = loc.Z + randomFloat;
		ptr->RelativeTransform.SetLocation(loc);
	}
	return;
}

#pragma optimize("", on)

IMPLEMENT_GAME_MODULE(FNoZFightingModule, NoZFighting);