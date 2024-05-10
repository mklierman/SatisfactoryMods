#include "DT_RCO.h"
#include <Net/UnrealNetwork.h>

void UDT_RCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UDT_RCO, bDummy);


}

void UDT_RCO::Server_MLBMerge_Implementation(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* groupLeaderOne, ALBBuild_ModularLoadBalancer* groupLeaderTwo)
{
	DigbyTool->MergeGroups(groupLeaderOne, groupLeaderTwo);

}

void UDT_RCO::Server_MLBSplit_Implementation(ADT_EquipDigbyTool * DigbyTool, const TArray< ALBBuild_ModularLoadBalancer*>& modulesToBeSplit)
{
	DigbyTool->SplitGroups(modulesToBeSplit);
}

void UDT_RCO::Server_SetFirstGroupLeader_Implementation(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* module)
{
	DigbyTool->SetFirstGroupLeader(module);
}

void UDT_RCO::Server_SetSecondGroupLeader_Implementation(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* module)
{
	DigbyTool->SetSecondGroupLeader(module);
}

void UDT_RCO::Server_AddModuleToBeSplit_Implementation(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* module)
{
	DigbyTool->AddModuleToBeSplit(module);
}

void UDT_RCO::Server_RemoveModuleToBeSplit_Implementation(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* module)
{
	DigbyTool->RemoveModuleToBeSplit(module);
}

void UDT_RCO::Server_ResetStuff_Implementation(ADT_EquipDigbyTool* DigbyTool)
{
	DigbyTool->ResetStuff();
}

void UDT_RCO::Server_HighlightGroup_Implementation(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* groupLeader)
{
	DigbyTool->HighlightGroup(groupLeader);
}

void UDT_RCO::Server_UnHighlightGroup_Implementation(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* groupLeader)
{
	DigbyTool->UnHighlightGroup(groupLeader);
}

void UDT_RCO::Server_UnHighlightAll_Implementation(ADT_EquipDigbyTool* DigbyTool)
{
	DigbyTool->UnHighlightAll();
}

void UDT_RCO::Server_HighlightModule_Implementation(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* module, UMaterialInterface* holoMaterial)
{
	DigbyTool->HighlightModule(module, holoMaterial);
}

void UDT_RCO::Server_UnHighlightModule_Implementation(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* module)
{
	DigbyTool->UnHighlightModule(module);
}
