#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LoadBalancers_Log, Display, All);

class FLoadBalancersModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }
};