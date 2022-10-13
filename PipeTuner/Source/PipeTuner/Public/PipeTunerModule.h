#pragma once

#include "Patching/NativeHookManager.h"
#include "Modules/ModuleManager.h"
#include "Hologram/FGPipelinePumpHologram.h"
#include "Hologram/FGPipelineAttachmentHologram.h"

class FPipeTunerModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	FTimerManager MyTimerManager;

	void SpawnPumpHG();

	TArray<AFGPipelinePumpHologram*> ActiveHolograms;

	TMap< AFGPipelinePumpHologram*, FTimerHandle> ActiveHologramTimers;
};