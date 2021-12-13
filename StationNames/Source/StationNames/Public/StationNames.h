// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(StationNames_Log, Display, All);
class FStationNamesModule : public FDefaultGameModuleImpl
{
public:

	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }
};
