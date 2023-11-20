#pragma once

#include "Modules/ModuleManager.h"
#include "Hologram/FGFactoryBuildingHologram.h"
#include "Hologram/FGBlueprintHologram.h"
#include "FGClearanceComponent.h"

class FUniversalSoftClearanceModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	void SetupClearance(AFGHologram* self, class UFGClearanceComponent* clearanceComponent);

	void GenerateCollisionObjects(AFGBlueprintHologram* self, const TArray< AFGBuildable* >& buildables);
};