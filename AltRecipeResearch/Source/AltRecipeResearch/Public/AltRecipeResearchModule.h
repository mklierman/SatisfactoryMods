#pragma once

#include "Modules/ModuleManager.h"

class FAltRecipeResearchModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }
};