#pragma once

#include "Modules/ModuleManager.h"
#include "Hologram/FGHologram.h"
#include "Hologram/FGFactoryBuildingHologram.h"
#include "FGBeamHologram.h"
#include "Equipment/FGBuildGunBuild.h"

class FLongerBeamsModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	void ScrollHologram(AFGBeamHologram* self, int32 delta);

	TArray<AFGBeamHologram*> ScrollingBeams;
};