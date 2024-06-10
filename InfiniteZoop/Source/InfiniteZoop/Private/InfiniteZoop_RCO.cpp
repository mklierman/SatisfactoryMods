


#include "InfiniteZoop_RCO.h"
#include "Registry/RemoteCallObjectRegistry.h"
#include <Logging/StructuredLog.h>

UInfiniteZoop_RCO::UInfiniteZoop_RCO()
{

}


//void UInfiniteZoop_RCO::GetZoopAmount_Implementation(UWorld* world, const int& OutCurrentZoopAmount)
//{
//	//USubsystemActorManager* SubsystemActorManager = world->GetSubsystem<USubsystemActorManager>();
//	//AInfiniteZoopSubsystem* zoopSubsystem = SubsystemActorManager->GetSubsystemActor<AInfiniteZoopSubsystem>();
//
//	//OutCurrentZoopAmount = zoopSubsystem->currentZoopAmount;
//	//OutZoopCorners = zoopSubsystem->zoopCorners;
//}

void UInfiniteZoop_RCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInfiniteZoop_RCO, DummyReplicatedField);
}

void UInfiniteZoop_RCO::SetHologramMaxZoop_Implementation(AFGFactoryBuildingHologram* hologram, int newMaxZoop)
{
	USubsystemActorManager* SubsystemActorManager = GetWorld()->GetSubsystem<USubsystemActorManager>();
	AInfiniteZoopSubsystem* zoopSubsystem = SubsystemActorManager->GetSubsystemActor<AInfiniteZoopSubsystem>();
	zoopSubsystem->currentZoopAmount = newMaxZoop;
	if (hologram)
	{
		hologram->mMaxZoopAmount = newMaxZoop;
		//UE_LOGFMT(InfiniteZoop_Log, Display, "SetHologramMaxZoop_Implementation {0}", newMaxZoop);
	}
}

//void UInfiniteZoop_RCO::GetZoopCorners_Implementation(UWorld* world, bool const& OutZoopCorners)
//{
//
//}
