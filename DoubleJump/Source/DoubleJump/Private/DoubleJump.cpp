// Copyright Epic Games, Inc. All Rights Reserved.

#include "DoubleJump.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FGCharacterPlayer.h"
#include "Patching/NativeHookManager.h"
#include "SessionSettings/SessionSettingsManager.h"
#include "UObject/UObjectIterator.h"

void FDoubleJumpModule::StartupModule()
{
	JumpAmountChangedDelegate.BindRaw(this, &FDoubleJumpModule::OnJumpAmountChanged);

	AFGCharacterPlayer* CharacterCDO = GetMutableDefault<AFGCharacterPlayer>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGCharacterPlayer::BeginPlay, CharacterCDO, [this](AFGCharacterPlayer* Player)
	{
		ApplyJumpAmount(Player);
	});

	WorldInitHandle = FWorldDelegates::OnPostWorldInitialization.AddLambda([this](UWorld* World, const UWorld::InitializationValues)
	{
		SubscribeToJumpAmount(World);
	});
}

void FDoubleJumpModule::ShutdownModule()
{
	FWorldDelegates::OnPostWorldInitialization.Remove(WorldInitHandle);

	if (!GEngine)
	{
		return;
	}

	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (UWorld* World = Context.World())
		{
			if (USessionSettingsManager* SessionSettings = World->GetSubsystem<USessionSettingsManager>())
			{
				SessionSettings->UnsubscribeToOptionUpdate(TEXT("DoubleJump.JumpAmount"), JumpAmountChangedDelegate);
			}
		}
	}

	JumpAmountChangedDelegate.Unbind();
}

void FDoubleJumpModule::ApplyJumpAmount(AFGCharacterPlayer* Player) const
{
	if (!Player)
	{
		return;
	}

	if (USessionSettingsManager* SessionSettings = Player->GetWorld()->GetSubsystem<USessionSettingsManager>())
	{
		Player->JumpMaxCount = FMath::RoundToInt(SessionSettings->GetFloatOptionValue(TEXT("DoubleJump.JumpAmount")));
	}
}

void FDoubleJumpModule::OnJumpAmountChanged(FString SettingId, FVariant Value)
{
	const int32 JumpAmount = FMath::RoundToInt(Value.GetValue<float>());
	for (TObjectIterator<AFGCharacterPlayer> It; It; ++It)
	{
		if (It->GetWorld() && It->GetWorld()->IsGameWorld())
		{
			It->JumpMaxCount = JumpAmount;
		}
	}
}

void FDoubleJumpModule::SubscribeToJumpAmount(UWorld* World)
{
	if (!World)
	{
		return;
	}

	if (USessionSettingsManager* SessionSettings = World->GetSubsystem<USessionSettingsManager>())
	{
		SessionSettings->SubscribeToOptionUpdate(TEXT("DoubleJump.JumpAmount"), JumpAmountChangedDelegate);
	}
}

IMPLEMENT_MODULE(FDoubleJumpModule, DoubleJump)
