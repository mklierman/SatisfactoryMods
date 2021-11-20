#pragma once

#include "Modules/ModuleManager.h"
#include "InfiniteZoopSubsystem.h"


DECLARE_LOG_CATEGORY_EXTERN(InfiniteZoop_Log, Display, All);
class FInfiniteZoopModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	//AInfiniteZoopSubsystem* zoopSubsystem;
};