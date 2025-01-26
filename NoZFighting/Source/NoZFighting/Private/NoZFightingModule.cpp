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
	//void AAbstractInstanceManager::SetInstanced(AActor * OwnerActor, const FTransform & ActorTransform, const FInstanceData & InstanceData, FInstanceHandle * &OutHandle, bool bInitializeHidden)
	SUBSCRIBE_METHOD(AAbstractInstanceManager::SetInstanced, [=](auto& scope, AAbstractInstanceManager* self, AActor* OwnerActor, const FTransform& ActorTransform, const FInstanceData& InstanceData, FInstanceHandle*& OutHandle, bool bInitializeHidden)
		{
			//scope(self, OwnerActor, ActorTransform, InstanceData, OutHandle, bInitializeHidden);
			SetInstanced(self, OwnerActor, ActorTransform, InstanceData, OutHandle, bInitializeHidden);
		});

	//AFGBuildable* bcdo = GetMutableDefault<AFGBuildable>();
	//SUBSCRIBE_METHOD_VIRTUAL(AFGBuildable::BeginPlay, bcdo, [=](auto& scope, AFGBuildable* self)
	//	{
	//		auto transform = self->GetActorTransform();
	//		OnConstruction(self, transform);
	//	});

	//int32 AFGLightweightBuildableSubsystem::AddFromBuildable(AFGBuildable * buildable, AActor * buildEffectInstigator, AFGBlueprintProxy * blueprintProxy) { return int32(); }
	//SUBSCRIBE_METHOD(AFGLightweightBuildableSubsystem::AddFromBuildable, [=](auto& scope, AFGLightweightBuildableSubsystem* self, AFGBuildable* buildable, AActor* buildEffectInstigator, AFGBlueprintProxy* blueprintProxy)
	//	{
	//		OnConstruction(buildable, buildable->GetActorTransform());
	//	});
	//AFGBuildable::HandleLightweightAddition
	//SUBSCRIBE_METHOD(AFGBuildable::HandleLightweightAddition, [=](auto& scope, AFGBuildable* self)
	//	{
	//		OnConstruction(self, self->GetActorTransform());
	//	});
	//
	//AFGBuildableHologram* bhgcdo = GetMutableDefault<AFGBuildableHologram>();
	//SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableHologram::Construct, bhgcdo, [=](AActor* actor, AFGBuildableHologram* self, TArray< AActor* >& out_children, FNetConstructionID netConstructionID)
	//	{
	//		auto buildable = Cast<AFGBuildable>(actor);
	//		auto transform = buildable->GetActorTransform();
	//		OnConstruction(buildable, transform);
	//	});
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

void FNoZFightingModule::SetInstanced(AAbstractInstanceManager* manager, AActor* OwnerActor, const FTransform& ActorTransform, const FInstanceData& InstanceData, FInstanceHandle*& OutHandle, bool bInitializeHidden)
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

	if (Name.ToString().ToLower().Contains("foundation"))
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
	return;

	//FInstanceComponentData* InstanceEntry = manager->InstanceMap.Find(Name);
	//FInstanceData* ptr;
	//ptr = (FInstanceData*)(&InstanceData);
	auto idk = manager->GetOwnerByHandle(*OutHandle);
	auto lwbs = Cast<AFGLightweightBuildableSubsystem>(idk);
	FLightweightBuildableInstanceRef out_buildableDescriptor;
	lwbs->ResolveLightweightInstance(*OutHandle, out_buildableDescriptor);
	if (lwbs)
	{
		for (auto active : lwbs->mActiveInstanceConverters)
		{
			auto buildFoundation = Cast<AFGBuildableFoundation>(active.Instigator);
			auto buildWalkway = Cast<AFGBuildableWalkway>(active.Instigator);
			auto holoFoundation = Cast<AFGFoundationHologram>(active.Instigator);
			if (buildFoundation || buildWalkway || holoFoundation)
			{
				auto config = FNoZFighting_ConfigStruct::GetActiveConfig(OwnerActor->GetWorld());
				auto min = config.FoundationMin;
				auto max = config.FoundationMax;
				float randomFloat = UKismetMathLibrary::RandomFloatInRange(min, max);
				//ptr->PositionOffset = FVector(0, 0, randomFloat);
				auto loc = ptr->RelativeTransform.GetLocation();
				loc.Z = loc.Z + randomFloat;
				ptr->RelativeTransform.SetLocation(loc);
				return;
			}
			else if (auto buildWall = Cast<AFGBuildableWall>(OwnerActor))
			{
				auto config = FNoZFighting_ConfigStruct::GetActiveConfig(OwnerActor->GetWorld());
				auto min = config.WallMin;
				auto max = config.WallMax;
				float randomFloat = UKismetMathLibrary::RandomFloatInRange(min, max);
				ptr->PositionOffset = FVector(randomFloat, 0, 0);
				ptr->bApplyRandomOffsetOnInstance = true;
				return;
			}
		}
	}
	else if (lwbs)
	{
		auto config = FNoZFighting_ConfigStruct::GetActiveConfig(OwnerActor->GetWorld());
		auto min = config.FoundationMin;
		auto max = config.FoundationMax;
		float randomFloat = UKismetMathLibrary::RandomFloatInRange(min, max);
		//ptr->PositionOffset = FVector(0, 0, randomFloat);
		auto loc = ptr->RelativeTransform.GetLocation();
		loc.Z = loc.Z + randomFloat;
		ptr->RelativeTransform.SetLocation(loc);

	}
}

#pragma optimize("", on)

IMPLEMENT_GAME_MODULE(FNoZFightingModule, NoZFighting);