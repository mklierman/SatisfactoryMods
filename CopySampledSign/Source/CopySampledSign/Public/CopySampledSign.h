// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include <Logging/StructuredLog.h>
#include "Buildables/FGBuildable.h"
#include "Patching/NativeHookManager.h"
#include "Hologram/FGBuildableHologram.h"
#include "Buildables/FGBuildableWidgetSign.h"
#include "FGSignTypes.h"
#include "FGCharacterPlayer.h"
#include "CSS_ActorComponent.h"

class FCopySampledSignModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void AddBuildable(class AFGBuildable* buildable, const AFGBuildableHologram* hologram);
};
