#include "PardonMeModule.h"
#include "Patching/NativeHookManager.h"
#include "Patching/BlueprintHookManager.h"
#include "Patching/BlueprintHookHelper.h"
#include "AI/FGAISystem.h"
#include "Creature/Enemy/FGCrabHatcher.h"
#include "FGGameMode.h"
#include "Creature/Actions/FGCreatureActionCharge.h"

void GameModePostLogin(CallScope<void(*)(AFGGameMode*, APlayerController*)>& scope, AFGGameMode* gm,
	APlayerController* pc)
{
#if !WITH_EDITOR
	UClass* SomeClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Character/Creature/Enemy/CrabHatcher/Char_CrabHatcher.Char_CrabHatcher_C"));
	UFunction* SomeFunc = SomeClass->FindFunctionByName(TEXT("StartNewWave"));
	UBlueprintHookManager* HookManager = GEngine->GetEngineSubsystem<UBlueprintHookManager>();
	HookManager->HookBlueprintFunction(SomeFunc, [](FBlueprintHookHelper& helper) {
		// Skip the normal function body -> prevent any crabs from being spawned
		helper.JumpToFunctionReturn();
		}, EPredefinedHookOffset::Start);
#endif
}

void FPardonMeModule::StartupModule() {
#if !WITH_EDITOR
	AFGGameMode* LocalGameMode = GetMutableDefault<AFGGameMode>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGGameMode::PostLogin, LocalGameMode, &GameModePostLogin)
#endif
}


IMPLEMENT_GAME_MODULE(FPardonMeModule, PardonMe);
