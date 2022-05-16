#pragma once

#include "Modules/ModuleManager.h"
#include "AI/FGAISystem.h"

class FPardonMeModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }
};