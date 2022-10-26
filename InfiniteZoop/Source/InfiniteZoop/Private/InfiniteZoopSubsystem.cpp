#include "InfiniteZoopSubsystem.h"

AInfiniteZoopSubsystem::AInfiniteZoopSubsystem()
{
    ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnServer_Replicate;
}

void AInfiniteZoopSubsystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInfiniteZoopSubsystem, currentZoopAmount);
}