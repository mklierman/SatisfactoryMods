#include "LoadBalancersModule.h"
#include "Hologram/FGBuildableHologram.h"
#include "Patching/NativeHookManager.h"

//DEFINE_LOG_CATEGORY(LoadBalancers_Log);

void FLoadBalancersModule::StartupModule() {

	//AFGBuildableHologram* bhgCDO = GetMutableDefault<AFGBuildableHologram>();
#if !WITH_EDITOR
	//virtual AActor* Construct(TArray< AActor* >&out_children, FNetConstructionID netConstructionID) override;
	//SUBSCRIBE_METHOD_VIRTUAL(AFGBuildableHologram::Construct, bhgCDO, [](auto scope, AFGBuildableHologram* self, TArray< AActor* >& out_children, FNetConstructionID netConstructionID)
	//	{
	//		if (self->GetSnappedBuilding())
	//		{
	//			auto sb = self->GetSnappedBuilding()->GetName();
	//			UE_LOG(LoadBalancers_Log, Display, TEXT("Snapped Building: %s"), *sb);
	//			//FDebug::DumpStackTraceToLog(ELogVerbosity::Display);
	//		}
	//	});
#endif
}


IMPLEMENT_GAME_MODULE(FLoadBalancersModule, LoadBalancers);