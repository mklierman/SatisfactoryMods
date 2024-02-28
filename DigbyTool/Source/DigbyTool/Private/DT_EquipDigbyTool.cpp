#include "DT_EquipDigbyTool.h"

void ADT_EquipDigbyTool::SetFirstGroupLeader(ALBBuild_ModularLoadBalancer* module)
{
	if (module)
	{
		firstGroupLeader = module;
		HighlightGroup(true);
	}
}

void ADT_EquipDigbyTool::SetSecondGroupLeader(ALBBuild_ModularLoadBalancer* module)
{
	if (module)
	{
		secondGroupLeader = module;
		HighlightGroup(false);
	}
}

void ADT_EquipDigbyTool::AddModuleToBeSplit(ALBBuild_ModularLoadBalancer* module)
{
	if (module)
	{
		modulesToSplit.Add(module);
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

void ADT_EquipDigbyTool::MergeGroups()
{
	currentToolError = EToolError::None;
	if (firstGroupLeader && secondGroupLeader)
	{
		if (firstGroupLeader->IsLeader())
		{
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

					ResetStuff();
				}
			}
		}
	}
}

void ADT_EquipDigbyTool::SplitGroups()
{
	currentToolError = EToolError::None;
	if (modulesToSplit.Num() > 0)
	{
		//Remove balancers from their groups
		for (auto balancer : modulesToSplit)
		{
			balancer->RemoveGroupModule();
		}

		//Set new leader and add balancers
		ALBBuild_ModularLoadBalancer* newLeader = modulesToSplit[0];
		for (auto balancer : modulesToSplit)
		{
			balancer->GroupLeader = newLeader;
			balancer->InitializeModule();
		}
		ResetStuff();
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
	splitBalancers.Empty();
	modulesToSplit.Empty();
}

void ADT_EquipDigbyTool::HighlightGroup(bool isFirstGroup)
{
	if (isFirstGroup && firstGroupLeader)
	{
		if (highlightedFirstGroupLeader)
		{
			UnHighlightGroup(true);
		}

		for (auto module : firstGroupLeader->GetGroupModules())
		{
			HighlightModule(module, firstGroupHoloMaterial);
		}
		highlightedFirstGroupLeader = firstGroupLeader;
	}
	else if (secondGroupLeader)
	{
		if (highlightedSecondGroupLeader)
		{
			UnHighlightGroup(false);
		}

		for (auto module : secondGroupLeader->GetGroupModules())
		{
			HighlightModule(module, secondGroupHoloMaterial);
		}
		highlightedSecondGroupLeader = secondGroupLeader;
	}
}

void ADT_EquipDigbyTool::UnHighlightGroup(bool isFirstGroup)
{
	if (isFirstGroup && highlightedFirstGroupLeader)
	{
		for (auto module : highlightedFirstGroupLeader->GetGroupModules())
		{
			UnHighlightModule(module);
		}
		highlightedFirstGroupLeader = nullptr;
	}
	else if (highlightedSecondGroupLeader)
	{
		for (auto module : highlightedSecondGroupLeader->GetGroupModules())
		{
			UnHighlightModule(module);
		}
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
	else if (highlightedSecondGroupLeader)
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
