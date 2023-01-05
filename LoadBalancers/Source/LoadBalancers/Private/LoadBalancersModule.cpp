#include "LoadBalancersModule.h"
#include "LBBuild_ModularLoadBalancer.h"
#include "Hologram/FGBuildableHologram.h"
#include "Patching/NativeHookManager.h"
#include "FGFactoryConnectionComponent.h"
#include "FGAttachmentPoint.h"
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
		UFGAttachmentPointType* LocalAP = GetMutableDefault<UFGAttachmentPointType>();
		SUBSCRIBE_METHOD_VIRTUAL(UFGAttachmentPointType::CanAttach, LocalAP, [=](auto& scope, const UFGAttachmentPointType* self, const struct FFGAttachmentPoint& point, const struct FFGAttachmentPoint& targetPoint)
			{
				if (auto targetBalancer = Cast< ALBBuild_ModularLoadBalancer>(targetPoint.Owner))
				{
					if (targetPoint.Type == point.Type )
					{
						if (targetPoint.Type->GetName() == "AP_LoadBalancerEnd_C" || targetPoint.Type->GetName() == "AP_LoadBalancerEnd2_C")
						{
							return;
						}
						scope.Override(false);
					}
				}
			});
}

IMPLEMENT_GAME_MODULE(FLoadBalancersModule, LoadBalancers);