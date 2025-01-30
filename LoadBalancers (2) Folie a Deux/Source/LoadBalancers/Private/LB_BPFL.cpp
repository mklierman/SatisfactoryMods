


#include "LB_BPFL.h"
#include <Logging/StructuredLog.h>

void ULB_BPFL::SortStringArray(UPARAM(ref)TArray<FString>& Array_To_Sort, TArray<FString>& Sorted_Array)
{
	Array_To_Sort.Sort();

	Sorted_Array = Array_To_Sort;
}

void ULB_BPFL::MergeBalancerGroups(ALBBuild_ModularLoadBalancer* groupLeaderOne, ALBBuild_ModularLoadBalancer* groupLeaderTwo)
{
	//UE_LOGFMT(LogTemp, Display, "ULB_BPFL::MergeBalancerGroups");
	if (groupLeaderOne && groupLeaderTwo)
	{
		//UE_LOGFMT(LogTemp, Display, "Group leaders aren't null");
		if (groupLeaderOne->IsLeader())
		{
			TArray< ALBBuild_ModularLoadBalancer*> balancersToMove = groupLeaderTwo->GetGroupModules();
			if (balancersToMove.Num() > 0)
			{
				for (ALBBuild_ModularLoadBalancer* ModularLoadBalancer : balancersToMove)
				{
					if (ModularLoadBalancer)
					{
						ModularLoadBalancer->RemoveGroupModule();
						ModularLoadBalancer->GroupLeader = groupLeaderOne;
						ModularLoadBalancer->InitializeModule();
						ModularLoadBalancer->SetCustomizationData_Implementation(groupLeaderOne->GetCustomizationData_Implementation());
					}
				}
				groupLeaderOne->ApplyLeader();
			}
		}
	}
}

void ULB_BPFL::SplitBalancers(TArray<ALBBuild_ModularLoadBalancer*> modulesToBeSplit)
{
	//UE_LOGFMT(LogTemp, Display, "ULB_BPFL::SplitBalancers");
	if (modulesToBeSplit.Num() > 0)
	{
		//UE_LOGFMT(LogTemp, Display, "modulesToBeSplit num: {0}", modulesToBeSplit.Num());
		TArray<ALBBuild_ModularLoadBalancer*> leaders;

		//Remove balancers from their groups
		for (auto balancer : modulesToBeSplit)
		{
			leaders.Add(balancer->GroupLeader);
			balancer->RemoveGroupModule();
		}

		//Set new leader and add balancers
		ALBBuild_ModularLoadBalancer* newLeader = nullptr;
		for (auto balancer : modulesToBeSplit)
		{
			if (!balancer->IsPendingKillOrUnreachable())
			{
				if (balancer->GroupLeader == nullptr && newLeader == nullptr)
				{
					newLeader = balancer;
					balancer->ApplyLeader();
				}
				balancer->GroupLeader = newLeader;
				balancer->InitializeModule();
				balancer->SetCustomizationData_Implementation(newLeader->GetCustomizationData_Implementation());
			}
			//}
		}
	}
}
