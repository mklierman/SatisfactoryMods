// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "FGInventoryComponent.h"

class FPlayerStackSizerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	int32 MoveInventoryItems(UFGInventoryComponent* sourceComponent, const int32 sourceIdx, UFGInventoryComponent* destinationComponent, const int32 destinationIdx);
};
