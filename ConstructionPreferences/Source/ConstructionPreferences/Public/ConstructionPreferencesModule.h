#pragma once

#include "Modules/ModuleManager.h"
#include "FGCharacterPlayer.h"

DECLARE_LOG_CATEGORY_EXTERN(LogConstructionPreferences, Display, All);
class FConstructionPreferencesModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	float GetUseDistance(const AFGCharacterPlayer* self);
};