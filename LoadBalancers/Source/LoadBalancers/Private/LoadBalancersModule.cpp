#include "LoadBalancersModule.h"
#include "Hologram/FGBuildableHologram.h"
#include "Patching/NativeHookManager.h"
#include "FGFactoryConnectionComponent.h"
#include "Buildables/FGBuildable.h"
#include "LBBuild_ModularLoadBalancer.h"

//DEFINE_LOG_CATEGORY(LoadBalancers_Log);

void FLoadBalancersModule::StartupModule() {

	//AFGBuildableHologram* bhgCDO = GetMutableDefault<AFGBuildableHologram>();
#if !WITH_EDITOR
 	 //  AFGBuildable::GetNumFactoryOuputConnections
	SUBSCRIBE_METHOD(AFGBuildable::GetNumFactoryOuputConnections, [](auto scope, const AFGBuildable* self )
		{
			UE_LOG(LoadBalancers_Log, Display, TEXT("AFGBuildable::GetNumFactoryOuputConnections"));
			auto Balancer = Cast< ALBBuild_ModularLoadBalancer>(self);
			if (Balancer)
			{
				UE_LOG(LoadBalancers_Log, Display, TEXT("It was a balancer!"));
				scope.Override(2);
			}
		});
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
//	SetConnection(class UFGFactoryConnectionComponent* toComponent);
	/*SUBSCRIBE_METHOD(UFGFactoryConnectionComponent::SetConnection, [](auto scope, UFGFactoryConnectionComponent* self, class UFGFactoryConnectionComponent* toComponent)
		{
			if (toComponent == nullptr)
			{
			auto buildable = self->GetOuterBuildable();
			if (buildable)
			{
			auto balancer = Cast< ALBBuild_ModularLoadBalancer>(buildable);
			if (balancer)
			{
				if (balancer->GroupLeader)
				{
					if (balancer->GroupLeader->OutputMap.Contains(self))
					{
						balancer->GroupLeader->OutputMap[self] = 0;
					}
				}
			}
			}
			}
		});*/
#endif
}


IMPLEMENT_GAME_MODULE(FLoadBalancersModule, LoadBalancers);