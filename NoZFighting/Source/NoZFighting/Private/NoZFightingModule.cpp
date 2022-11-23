#include "NoZFightingModule.h"
#include "Patching/NativeHookManager.h"
#include "Hologram/FGBuildableHologram.h"
#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildableFoundation.h"
#include "Buildables/FGBuildableWall.h"
#include "Kismet/KismetMathLibrary.h"


void FNoZFightingModule::StartupModule() {

#if !WITH_EDITOR
	AFGBuildable* bcdo = GetMutableDefault<AFGBuildable>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildable::OnConstruction, bcdo, [](AFGBuildable* self, const FTransform& transform)
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
				auto instances = buildFoundation->mInstanceHandles;
				for (auto inst : instances)
				{
					auto sMesh = inst->GetInstanceComponent();
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
					auto instances = buildWall->mInstanceHandles;
					for (auto inst : instances)
					{
						auto sMesh = inst->GetInstanceComponent();
						float randomFloat = UKismetMathLibrary::RandomFloatInRange(-0.35, 0.35);
						sMesh->AddRelativeLocation(FVector(0.0, 0.0, randomFloat));
					}
				}
			}
		});
#endif
}


IMPLEMENT_GAME_MODULE(FNoZFightingModule, NoZFighting);