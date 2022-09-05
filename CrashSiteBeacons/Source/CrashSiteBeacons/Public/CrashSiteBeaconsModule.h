#pragma once

#include "Modules/ModuleManager.h"

class FCrashSiteBeaconsModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }


};