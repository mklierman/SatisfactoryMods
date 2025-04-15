#pragma once

#include "Buildables/FGBuildable.h"
#include "AbstractInstanceManager.h"
#include "AbstractInstanceInterface.h"
#include "InstanceData.h"
#include "AbstractInstance.h"
#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "AbstractInstanceManager.h"
#include "FGColoredInstanceMeshProxy.h"
#include "Hologram/FGBuildableHologram.h"
#include "Modules/ModuleManager.h"
#include "NoZFighting_ConfigStruct.h"

class FNoZFightingModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	void OnConstruction(AFGBuildable* self, const FTransform& transform);

	void SetInstanced(AAbstractInstanceManager* manager, AActor* OwnerActor, const FTransform& ActorTransform, const FInstanceData& InstanceData, FInstanceOwnerHandlePtr& OutHandle, bool bInitializeHidden);
};