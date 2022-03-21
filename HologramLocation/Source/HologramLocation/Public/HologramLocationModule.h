#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LocAndRot_Log, Display, All);

class FHologramLocationModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	void AddLocRes(TArray<FString>& newLocs);
};