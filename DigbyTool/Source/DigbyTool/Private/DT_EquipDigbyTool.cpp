#include "DT_EquipDigbyTool.h"

void ADT_EquipDigbyTool::MergeGroups()
{
	currentToolError = EToolError::None;
	if (firstGroupLeader && secondGroupLeader)
	{
		if (firstGroupLeader->IsLeader())
		{
			if (firstGroupLeader->HasOverflowModule() && secondGroupLeader->HasOverflowModule())
			{
				//Send message about overflows
				currentToolError = EToolError::MergeOverflows;
				return;
			}

			TArray< ALBBuild_ModularLoadBalancer*> balancersToMove = secondGroupLeader->GetGroupModules();
			if (balancersToMove.Num() > 0)
			{
				if (HasAuthority())
				{
					secondGroupLeader->GroupLeader = firstGroupLeader;

					for (ALBBuild_ModularLoadBalancer* ModularLoadBalancer : balancersToMove)
					{
						if (ModularLoadBalancer)
						{
							ModularLoadBalancer->GroupLeader = firstGroupLeader;
							ModularLoadBalancer->mNormalLoaderData = firstGroupLeader->mNormalLoaderData;
							ModularLoadBalancer->ApplyGroupModule();
						}
					}
				}
			}
		}
	}
}

void ADT_EquipDigbyTool::SplitGroups()
{
	currentToolError = EToolError::None;
	if (splitBalancers.Num() > 0)
	{
		//Make sure we aren't trying to add multiple overflows to the new group
		int numOverflows = 0;
		for (auto balancer : splitBalancers)
		{
			if (balancer->IsOverflowModule())
			{
				numOverflows++;
				if (numOverflows > 1)
				{
					//Send message about overflows
					currentToolError = EToolError::SplitOverflows;
					return;
				}
			}
		}

		//Remove balancers from their groups
		for (auto balancer : splitBalancers)
		{
			balancer->RemoveGroupModule();
		}

		//Set new leader and add balancers
		ALBBuild_ModularLoadBalancer* newLeader = splitBalancers[0];
		for (auto balancer : splitBalancers)
		{
			balancer->GroupLeader = newLeader;
			balancer->InitializeModule();
		}
	}
}

EToolMode ADT_EquipDigbyTool::GetCurrentToolMode()
{
	return currentToolMode;
}

void ADT_EquipDigbyTool::SetCurrentToolMode(EToolMode newMode)
{
	currentToolMode = newMode;
}
