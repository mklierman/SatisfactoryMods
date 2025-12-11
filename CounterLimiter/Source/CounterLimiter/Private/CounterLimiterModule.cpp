#include "CounterLimiterModule.h"
#include "Patching/NativeHookManager.h"
#include "CL_CounterLimiter.h"
#include "Hologram/FGStandaloneSignHologram.h"
#include "FGIconLibrary.h"
#include "FGGameMode.h"
#include "CL_RCO.h"
#include "Buildables/FGBuildableWidgetSign.h"

DEFINE_LOG_CATEGORY(CounterLimiter_Log);

void FCounterLimiterModule::StartupModule() {
#if !WITH_EDITOR
	AFGStandaloneSignHologram* signHGCDO = GetMutableDefault<AFGStandaloneSignHologram>();

	AFGGameMode* LocalGameMode = GetMutableDefault<AFGGameMode>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGGameMode::PostLogin, LocalGameMode, [](auto& scope, AFGGameMode* gm, APlayerController* pc)
		{
			if (gm->HasAuthority() && !gm->IsMainMenuGameMode())
			{
				//Register RCO
				gm->RegisterRemoteCallObjectClass(UCL_RCO::StaticClass());
			}
		});
#endif
}


IMPLEMENT_GAME_MODULE(FCounterLimiterModule, CounterLimiter);