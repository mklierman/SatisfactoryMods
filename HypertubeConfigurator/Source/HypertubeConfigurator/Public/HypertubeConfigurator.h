// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include <FGCharacterMovementComponent.h>

DECLARE_LOG_CATEGORY_EXTERN(HypertubeConfigurator_Log, Display, All);

class FHypertubeConfiguratorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void SetHypertubeStuff(UFGCharacterMovementComponent* movementComp);
};
