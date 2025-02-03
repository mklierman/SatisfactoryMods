// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Hologram/FGHologram.h"
#include "FGConstructDisqualifier.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FIdenticalBuildableModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	bool AddDisqualifier(AFGHologram* self, TSubclassOf< class UFGConstructDisqualifier > disqualifier);
};
