// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FGOptionInterface.h"
#include "Modules/ModuleManager.h"

class AFGCharacterPlayer;
class UWorld;

class FDoubleJumpModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void ApplyJumpAmount(AFGCharacterPlayer* Player) const;
	void OnJumpAmountChanged(FString SettingId, FVariant Value);
	void SubscribeToJumpAmount(UWorld* World);

	FOnOptionUpdated JumpAmountChangedDelegate;
	FDelegateHandle WorldInitHandle;
};
