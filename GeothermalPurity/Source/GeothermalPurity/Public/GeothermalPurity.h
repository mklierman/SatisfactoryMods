// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class AFGResourceNodeGeyser;

DECLARE_LOG_CATEGORY_EXTERN(GeothermalPurity_Log, Display, All);

class FGeothermalPurityModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void SetGeyserPurity(AFGResourceNodeGeyser* geyser);
};
