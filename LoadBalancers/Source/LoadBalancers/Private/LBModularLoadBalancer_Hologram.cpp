

#include "LBModularLoadBalancer_Hologram.h"
#include "LoadBalancersModule.h"

DEFINE_LOG_CATEGORY(LoadBalancersHG_Log);
AActor* ALBModularLoadBalancer_Hologram::Construct(TArray<AActor*>& out_children, FNetConstructionID netConstructionID)
{
	AActor* constructedActor = Super::Construct(out_children, netConstructionID);
	ALBBuild_ModularLoadBalancer* newBalancer = Cast<ALBBuild_ModularLoadBalancer>(constructedActor);
	if (newBalancer)
	{
		ALBBuild_ModularLoadBalancer* SnappedBalancer = Cast<ALBBuild_ModularLoadBalancer>(this->GetSnappedBuilding());

		//Get subsystem
		UWorld* WorldObject = this->GetWorld();
		USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
		ALoadBalancers_Subsystem_CPP* subsystem = SubsystemActorManager->GetSubsystemActor<ALoadBalancers_Subsystem_CPP>();
		if (subsystem)
		{
			//Add newly constructed actor to subsytem arrays along with snapped building
			if (SnappedBalancer)
			{
				newBalancer->SnappedBalancer = SnappedBalancer;
				newBalancer->GroupLeader = SnappedBalancer->GroupLeader;
				if (newBalancer->GroupLeader)
				{
					for (auto conn : newBalancer->InputConnections)
					{
						newBalancer->GroupLeader->InputConnections.Add(conn);
						newBalancer->GroupLeader->InputQueue.Enqueue(conn);
					}
					for (auto conn : newBalancer->OutputMap)
					{
						newBalancer->GroupLeader->OutputMap.Add(conn);
					}
				}

				//for (FLBBalancerGroup group : subsystem->BalancerGroups)
				//{
				//	if (group.PrimaryBalancer == newBalancer->GroupLeader)
				//	{
				//		group.GroupBalancers.Add(newBalancer);
				//	}
				//}
				int32 currentInvSize = newBalancer->GroupLeader->Buffer->GetSizeLinear();
				newBalancer->GroupLeader->Buffer->Resize(currentInvSize + 2);
			}
			else
			{
				FLBBalancerGroup newGroup;
				newGroup.PrimaryBalancer = newBalancer;
				newGroup.GroupBalancers.Add(newBalancer);
				subsystem->BalancerGroups.Add(newGroup);
				newBalancer->GroupLeader = newBalancer;
			}
		}
	}

	return constructedActor;
}
