#pragma once

#include "Equipment/FGHoverPack.h"
#include "Modules/ModuleManager.h"

class FNoHoverPackDriftModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	void StopDrift(AFGHoverPack* self, float deltaTime);
};