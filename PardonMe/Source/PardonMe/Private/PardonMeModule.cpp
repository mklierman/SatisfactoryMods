#include "PardonMeModule.h"
#include "Patching/NativeHookManager.h"
#include "AI/FGAISystem.h"
#include "AI/FGEnemyController.h"

void FPardonMeModule::StartupModule() {
#if !WITH_EDITOR
	SUBSCRIBE_METHOD(AFGEnemyController::AddNewAggroTarget, [](auto& scope, AFGEnemyController* self, const TScriptInterface< IFGAggroTargetInterface > target)
	{
		scope.Cancel();
	});
#endif
}


IMPLEMENT_GAME_MODULE(FPardonMeModule, PardonMe);