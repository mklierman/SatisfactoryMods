#include "LoadBalancersModule.h"
#include "Hologram/FGBuildableHologram.h"
#include "Patching/NativeHookManager.h"
#include "FGFactoryConnectionComponent.h"
#include "FGGameMode.h"
#include "LBDefaultRCO.h"

//DEFINE_LOG_CATEGORY(LoadBalancers_Log);

void GameModePostLogin(CallScope<void(*)(AFGGameMode*, APlayerController*)>& scope, AFGGameMode* gm,
					   APlayerController* pc)
{
	if (gm->HasAuthority() && !gm->IsMainMenuGameMode())
	{
		//Register RCO
		gm->RegisterRemoteCallObjectClass(ULBDefaultRCO::StaticClass());
	}
}

void FLoadBalancersModule::StartupModule() {
#if !WITH_EDITOR
	// Hooking to register RCOs
	AFGGameMode* LocalGameMode = GetMutableDefault<AFGGameMode>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGGameMode::PostLogin, LocalGameMode, &GameModePostLogin)
#endif
}


IMPLEMENT_GAME_MODULE(FLoadBalancersModule, LoadBalancers);