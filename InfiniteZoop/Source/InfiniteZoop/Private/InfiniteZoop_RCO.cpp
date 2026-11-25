


#include "InfiniteZoop_RCO.h"
#include "InfiniteZoop_ClientSubsystem.h"
#include "Registry/RemoteCallObjectRegistry.h"
#include "Net/UnrealNetwork.h"
#include <Logging/StructuredLog.h>

UInfiniteZoop_RCO::UInfiniteZoop_RCO()
{

}

void UInfiniteZoop_RCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInfiniteZoop_RCO, DummyReplicatedField);
}

void UInfiniteZoop_RCO::SetHologramMaxZoop_Implementation(AFGBuildableHologram* hologram, int newMaxZoop)
{
	USubsystemActorManager* SubsystemActorManager = GetWorld()->GetSubsystem<USubsystemActorManager>();
	AInfiniteZoopSubsystem* zoopSubsystem = SubsystemActorManager->GetSubsystemActor<AInfiniteZoopSubsystem>();
	//zoopSubsystem->currentZoopAmount = newMaxZoop;
	if (hologram)
	{
		hologram->mMaxZoopAmount = newMaxZoop;
		//UE_LOGFMT(InfiniteZoop_Log, Display, "SetHologramMaxZoop_Implementation {0}", newMaxZoop);
	}
}

void UInfiniteZoop_RCO::SetDesiredZoop_Implementation(AFGBuildableHologram* hologram, FIntVector newZoop)
{
	if (hologram)
	{
		hologram->mDesiredZoop = newZoop;
	}
}

void UInfiniteZoop_RCO::SetTemporaryZoopAmount(int32 Amount)
{
	TemporaryZoopAmount = Amount;
	ClientSetTemporaryZoopAmount(Amount);
}

void UInfiniteZoop_RCO::ClientSetTemporaryZoopAmount_Implementation(int32 Amount)
{
	if (AInfiniteZoop_ClientSubsystem* Subsystem = AInfiniteZoop_ClientSubsystem::Get(this))
	{
		Subsystem->tempZoopAmount = Amount;
	}
}
