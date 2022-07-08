#include "PardonMeModule.h"
#include "Patching/NativeHookManager.h"
#include "Patching/BlueprintHookManager.h"
#include "Patching/BlueprintHookHelper.h"
#include "AI/FGAISystem.h"
#include "Creature/Enemy/FGCrabHatcher.h"
#include "FGGameMode.h"
//#include "AI/FGEnemyController.h"

void GameModePostLogin(CallScope<void(*)(AFGGameMode*, APlayerController*)>& scope, AFGGameMode* gm,
	APlayerController* pc)
{
#if !WITH_EDITOR
	UClass* SomeClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Character/Creature/Enemy/CrabHatcher/Char_CrabHatcher.Char_CrabHatcher_C"));
	UFunction* SomeFunc = SomeClass->FindFunctionByName(TEXT("StartNewWave"));
	UBlueprintHookManager* HookManager = GEngine->GetEngineSubsystem<UBlueprintHookManager>();
	HookManager->HookBlueprintFunction(SomeFunc, [](FBlueprintHookHelper& helper) {
		UObject* ctx = helper.GetContext(); // the object this function got called onto
		// do some nice stuff there
		helper.JumpToFunctionReturn();
		}, EPredefinedHookOffset::Start);
#endif
}

void FPardonMeModule::StartupModule() {
#if !WITH_EDITOR
	AFGGameMode* LocalGameMode = GetMutableDefault<AFGGameMode>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGGameMode::PostLogin, LocalGameMode, &GameModePostLogin)

	//SUBSCRIBE_METHOD(AFGEnemyController::AddNewAggroTarget, [](auto& scope, AFGEnemyController* self, const TScriptInterface< IFGAggroTargetInterface > target)
	//{
	//	scope.Cancel();
	//});

#endif
}


IMPLEMENT_GAME_MODULE(FPardonMeModule, PardonMe);