#include "DT_EquipDigbyTool.h"
#include "DT_RCO.h"

DEFINE_LOG_CATEGORY(DigbyTool_Log);
#pragma optimize("", off)
void ADT_EquipDigbyTool::SetFirstGroupLeader(ALBBuild_ModularLoadBalancer* module)
{
	if (module && module != secondGroupLeader)
	{
		firstGroupLeader = module;
		HighlightGroup(module);
	}
}

void ADT_EquipDigbyTool::SetSecondGroupLeader(ALBBuild_ModularLoadBalancer* module)
{
	if (module && module != firstGroupLeader)
	{
		secondGroupLeader = module;
		HighlightGroup(module);
	}
}

void ADT_EquipDigbyTool::AddModuleToBeSplit(ALBBuild_ModularLoadBalancer* module)
{
	if (module)
	{
		modulesToSplit.AddUnique(module);
		HighlightModule(module, firstGroupHoloMaterial);
	}
}

void ADT_EquipDigbyTool::RemoveModuleToBeSplit(ALBBuild_ModularLoadBalancer* module)
{
	if (module && modulesToSplit.Contains(module))
	{
		modulesToSplit.Remove(module);
		UnHighlightModule(module);
	}
}

void ADT_EquipDigbyTool::MergeGroups(ALBBuild_ModularLoadBalancer* groupLeaderOne, ALBBuild_ModularLoadBalancer* groupLeaderTwo)
{
	if (HasAuthority())
	{
		if (groupLeaderOne && groupLeaderTwo)
		{
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
							//ModularLoadBalancer->mNormalLoaderData = firstGroupLeader->mNormalLoaderData;
							ModularLoadBalancer->InitializeModule();
						}
					}
					groupLeaderOne->ApplyLeader();
					groupLeaderTwo = nullptr;
					currentToolError = EToolError::None;
					UnHighlightAll();
				}
			}
		}
	}
	else
	{
		if (UDT_RCO* RCO = UDT_RCO::Get(GetWorld()))
		{
			RCO->Server_MLBMerge(this, groupLeaderOne, groupLeaderTwo);
			currentToolError = EToolError::None;
			UnHighlightAll();
		}
	}
	ResetStuff();
}

void ADT_EquipDigbyTool::SplitGroups(TArray< ALBBuild_ModularLoadBalancer*> modulesToBeSplit)
{
	if (HasAuthority())
	{
		if (modulesToBeSplit.Num() > 0)
		{
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
				}
			}
			currentToolError = EToolError::None;
			ResetStuff();
		}
	}
	else
	{
		if (UDT_RCO* RCO = UDT_RCO::Get(GetWorld()))
		{
			RCO->Server_MLBSplit(this, modulesToBeSplit);
			currentToolError = EToolError::None;
			ResetStuff();
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

void ADT_EquipDigbyTool::CycleToolMode()
{
	if (GetCurrentToolMode() == EToolMode::Split)
	{
		SetCurrentToolMode(EToolMode::Merge);
	}
	else
	{
		SetCurrentToolMode(EToolMode::Split);
	}
	ResetStuff();
}

void ADT_EquipDigbyTool::ResetStuff()
{
	currentToolError = EToolError::None;
	UnHighlightAll();
	firstGroupLeader = nullptr;
	secondGroupLeader = nullptr;
	modulesToSplit.Empty();
}

void ADT_EquipDigbyTool::HighlightGroup(ALBBuild_ModularLoadBalancer* groupLeader)
{
	if (groupLeader == firstGroupLeader)
	{
		if (highlightedFirstGroupLeader)
		{
			UnHighlightGroup(highlightedFirstGroupLeader);
		}
		for (auto module : groupLeader->GetGroupModules())
		{
			HighlightModule(module, firstGroupHoloMaterial);
		}
		highlightedFirstGroupLeader = groupLeader;
	}
	else if (groupLeader == secondGroupLeader)
	{
		if (highlightedSecondGroupLeader)
		{
			UnHighlightGroup(highlightedSecondGroupLeader);
		}

		for (auto module : groupLeader->GetGroupModules())
		{
			HighlightModule(module, secondGroupHoloMaterial);
		}
		highlightedSecondGroupLeader = groupLeader;
	}
}

void ADT_EquipDigbyTool::UnHighlightGroup(ALBBuild_ModularLoadBalancer* groupLeader)
{
	for (auto module : groupLeader->GetGroupModules())
	{
		UnHighlightModule(module);
	}
	if (highlightedFirstGroupLeader && highlightedFirstGroupLeader == groupLeader)
	{
		highlightedFirstGroupLeader = nullptr;
	}
	else if (highlightedSecondGroupLeader && highlightedSecondGroupLeader == groupLeader)
	{
		highlightedSecondGroupLeader = nullptr;
	}
}

void ADT_EquipDigbyTool::UnHighlightAll()
{
	if (highlightedFirstGroupLeader)
	{
		for (auto module : highlightedFirstGroupLeader->GetGroupModules())
		{
			UnHighlightModule(module);
		}
		highlightedFirstGroupLeader = nullptr;
	}
	if (highlightedSecondGroupLeader)
	{
		for (auto module : highlightedSecondGroupLeader->GetGroupModules())
		{
			UnHighlightModule(module);
		}
		highlightedSecondGroupLeader = nullptr;
	}
	for (auto module : modulesToSplit)
	{
		UnHighlightModule(module);
	}
	modulesToSplit.Empty();
}

void ADT_EquipDigbyTool::HighlightModule(ALBBuild_ModularLoadBalancer* module, UMaterialInterface* holoMaterial)
{
	if (module)
	{
		auto comp = module->GetComponentByClass(UFGColoredInstanceMeshProxy::StaticClass());
		if (auto mesh = Cast<UFGColoredInstanceMeshProxy>(comp))
		{
			mesh->SetMaterial(0, holoMaterial);
			mesh->SetMaterial(1, holoMaterial);
			mesh->SetInstanced(false);
			mesh->SetInstanced(true);
		}
	}
}

void ADT_EquipDigbyTool::UnHighlightModule(ALBBuild_ModularLoadBalancer* module)
{
	if (module)
	{
		auto comp = module->GetComponentByClass(UFGColoredInstanceMeshProxy::StaticClass());
		if (auto mesh = Cast<UFGColoredInstanceMeshProxy>(comp))
		{
			mesh->SetMaterial(0, mainMaterial);
			mesh->SetMaterial(1, secondaryMaterial);
			mesh->SetInstanced(false);
			mesh->SetInstanced(true);
		}
	}
}

int32 ADT_EquipDigbyTool::GetSplitCount()
{
	return modulesToSplit.Num();
}

int32 ADT_EquipDigbyTool::GetFirstGroupCount()
{
	if (firstGroupLeader)
	{
		return firstGroupLeader->GetGroupModules().Num();
	}
	return 0;
}

int32 ADT_EquipDigbyTool::GetSecondGroupCount()
{
	if (secondGroupLeader)
	{
		return secondGroupLeader->GetGroupModules().Num();
	}
	return 0;
}

#pragma optimize("", on)