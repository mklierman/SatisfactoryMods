#include "TestHooker_Subsystem.h"
#include <Logging/StructuredLog.h>
#include "Patching/NativeHookManager.h"
#include "FGCharacterPlayer.h"
#include "FGEmote.h"
#include "Patching/BlueprintHookManager.h"

DEFINE_LOG_CATEGORY(TestHooker_Log);

void ATestHooker_Subsystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (EmoteHook.IsValid())
		UnhookEmote();

	if (EmoteSFXHook.IsValid())
		UnhookEmoteSFX();

	if (JumpHook.IsValid())
		UnhookJump();

	Super::EndPlay(EndPlayReason);
}

void ATestHooker_Subsystem::HookEmote()
{
#if !WITH_EDITOR
	EmoteHook = SUBSCRIBE_METHOD(UFGEmote::OnPlayEmoteLocally, [](auto& scope, UFGEmote* emote)
		{
			UE_LOGFMT(TestHooker_Log, Display, "Played emote");
		});
#endif
}

void ATestHooker_Subsystem::UnhookEmote()
{
#if !WITH_EDITOR
	if (EmoteHook.IsValid())
		UNSUBSCRIBE_METHOD(UFGEmote::OnPlayEmoteLocally, EmoteHook);
#endif
}

void ATestHooker_Subsystem::HookEmoteSFX()
{
#if !WITH_EDITOR
	EmoteSFXHook = SUBSCRIBE_METHOD_AFTER(UFGEmote::PlayEmoteSFX, [](UFGEmote* emote, class UAkAudioEvent* event)
		{
			UE_LOGFMT(TestHooker_Log, Display, "Played emote SFX");
		});
#endif
}

void ATestHooker_Subsystem::UnhookEmoteSFX()
{
#if !WITH_EDITOR
	if (EmoteSFXHook.IsValid())
		UNSUBSCRIBE_METHOD(UFGEmote::PlayEmoteSFX, EmoteSFXHook);
#endif
}

void ATestHooker_Subsystem::HookJump()
{
#if !WITH_EDITOR
	AFGCharacterPlayer* cpCDO = GetMutableDefault<AFGCharacterPlayer>();
	JumpHook = SUBSCRIBE_METHOD_VIRTUAL(AFGCharacterPlayer::Jump, cpCDO, [](auto& scope, AFGCharacterPlayer* self)
		{
			UE_LOGFMT(TestHooker_Log, Display, "Jumped");
		});
#endif
}

void ATestHooker_Subsystem::UnhookJump()
{
#if !WITH_EDITOR
	if (JumpHook.IsValid())
		UNSUBSCRIBE_METHOD(AFGCharacterPlayer::Jump, JumpHook);
#endif
}

void ATestHooker_Subsystem::HookBPToggleFlashlight()
{
#if !WITH_EDITOR
	UClass* SomeClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Character/Player/Char_Player.Char_Player.Char_Player_C"));
	UFunction* SomeFunc = SomeClass->FindFunctionByName(TEXT("ToggleFlashlight"));
	UBlueprintHookManager* HookManager = GEngine->GetEngineSubsystem<UBlueprintHookManager>();

	HookManager->HookBlueprintFunction(SomeFunc, [](const FBlueprintHookHelper& HookHelper)
		{
			UE_LOGFMT(TestHooker_Log, Display, "Toggled flashlight");
		}, EPredefinedHookOffset::Start);
#endif
}

void ATestHooker_Subsystem::UnHookBPToggleFlashlight()
{

}

void ATestHooker_Subsystem::HookBPToggleInventory()
{
#if !WITH_EDITOR
	UClass* SomeClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Character/Player/Char_Player.Char_Player.Char_Player_C"));
	UFunction* SomeFunc = SomeClass->FindFunctionByName(TEXT("ToggleInventory"));
	UBlueprintHookManager* HookManager = GEngine->GetEngineSubsystem<UBlueprintHookManager>();

	HookManager->HookBlueprintFunction(SomeFunc, [](const FBlueprintHookHelper& HookHelper)
		{
			UE_LOGFMT(TestHooker_Log, Display, "Toggled inventory");
		}, EPredefinedHookOffset::Return);
#endif
}

void ATestHooker_Subsystem::UnHookBPToggleInventory()
{
}
