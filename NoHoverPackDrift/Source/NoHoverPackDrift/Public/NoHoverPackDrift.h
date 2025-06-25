// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Equipment/FGHoverPack.h"

class FNoHoverPackDriftModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void OnCharacterMovementModeChanged(AFGHoverPack* hoverpack);

	void SetDefaults(AFGHoverPack* hoverpack);
};
