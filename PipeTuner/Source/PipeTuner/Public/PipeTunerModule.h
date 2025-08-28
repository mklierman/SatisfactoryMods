#pragma once

#include "Patching/NativeHookManager.h"
#include "Modules/ModuleManager.h"
#include "Hologram/FGPipelinePumpHologram.h"
#include "Hologram/FGPipelineAttachmentHologram.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPipeTuner, Display, All);
class FPipeTunerModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }
};