//
//
//
//
//void ULB_RCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//	DOREPLIFETIME(ULB_RCO, Dummy);
//}

#include "LBDefaultRCO.h"

void ULBDefaultRCO::Server_SetFilteredItem_Implementation(ALBBuild_ModularLoadBalancer* Balancer,
	TSubclassOf<UFGItemDescriptor> ItemClass)
{
	if (Balancer)
	{
		Balancer->SetFilteredItem(ItemClass);
		Balancer->ForceNetUpdate();
	}
}

void ULBDefaultRCO::Server_CreateOutlineForActor_Implementation(ALBOutlineSubsystem* Subsystem, AActor* Actor)
{
	if (Subsystem && Actor)
	{
		Subsystem->MultiCast_CreateOutlineForActor(Actor);
		Subsystem->ForceNetUpdate();
	}
}

void ULBDefaultRCO::Server_ClearOutlines_Implementation(ALBOutlineSubsystem* Subsystem)
{
	if (Subsystem )
	{
		Subsystem->MultiCast_ClearOutlines();
		Subsystem->ForceNetUpdate();
	}
}

void ULBDefaultRCO::Server_SetOutlineColor_Implementation(ALBOutlineSubsystem* Subsystem, FLinearColor Color)
{
	if (Subsystem )
	{
		Subsystem->MultiCast_SetOutlineColor(Color);
		Subsystem->ForceNetUpdate();
	}
}

void ULBDefaultRCO::Server_ResetOutlineColor_Implementation(ALBOutlineSubsystem* Subsystem)
{
	if (Subsystem )
	{
		Subsystem->MultiCast_ResetOutlineColor();
		Subsystem->ForceNetUpdate();
	}
}

void ULBDefaultRCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ULBDefaultRCO, bDummy);
}

void ULBDefaultRCO::Server_RemoveFilteredItem_Implementation(ALBBuild_ModularLoadBalancer* Balancer,
	TSubclassOf<UFGItemDescriptor> ItemClass)
{
	if (Balancer)
	{
		Balancer->RemoveFilteredItem(ItemClass);
		Balancer->ForceNetUpdate();
	}
}
