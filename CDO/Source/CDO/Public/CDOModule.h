#pragma once

#include "Modules/ModuleManager.h"
#include "Creature/Enemy/FGEnemy.h"

DECLARE_LOG_CATEGORY_EXTERN(CDO_Log, Display, All);

class FCDOModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }
};