#include "LoadBalancersModule.h"
#include "Hologram/FGBuildableHologram.h"
#include "Patching/NativeHookManager.h"
#include "FGFactoryConnectionComponent.h"
#include "Buildables/FGBuildable.h"
#include "LBBuild_ModularLoadBalancer.h"

//DEFINE_LOG_CATEGORY(LoadBalancers_Log);

void FLoadBalancersModule::StartupModule() {

	//		UE_LOG(LoadBalancers_Log, Display, TEXT("AFGBuildable::GetNumFactoryOuputConnections"));
}


IMPLEMENT_GAME_MODULE(FLoadBalancersModule, LoadBalancers);