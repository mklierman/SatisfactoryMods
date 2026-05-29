#pragma once

#include "Modules/ModuleManager.h"
#include "FGAtmosphereVolume.h"
#include "Atmosphere/UFGBiome.h"

class FAtmosphereEditorModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	void AtmosphereVolueBeginPlay(AFGAtmosphereVolume* volume);
};