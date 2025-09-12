#pragma once

#include "Modules/ModuleManager.h"
#include "Hologram/FGFactoryBuildingHologram.h"
#include "Hologram/FGBlueprintHologram.h"
#include "Buildables/FGBuildableElevator.h"

class FUniversalSoftClearanceModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	//void SetupClearance(AFGHologram* self, class UFGClearanceComponent* clearanceComponent);

	void GenerateCollisionObjects(AFGBlueprintHologram* self, const TArray< AFGBuildable* >& buildables);
	void BeginPlay(AFGBuildable* self);
	void HBeginPlay(AFGHologram* self);

	void GetChildClearanceOutlineActors(const AFGBuildableElevator* self, TArray<AActor*>& actors);
};