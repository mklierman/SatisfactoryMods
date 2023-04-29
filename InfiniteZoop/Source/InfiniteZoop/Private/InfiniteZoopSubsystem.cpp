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

void AInfiniteZoopSubsystem::SetPublicZoopAmount(int x, int y, int z, bool foundation, bool verticalZoop)
{
	if (foundation && verticalZoop)
	{
		xAmount = -1;
		zAmount = -1;
		yAmount = -1;
		needsUpdate = true;
		return;
	}
	if (x == xAmount && y == yAmount && z == zAmount && foundation == isFoundation)
	{
		needsUpdate = false;
	}
	else
	{
		xAmount = x;
		yAmount = y;
		zAmount = z;
		isFoundation = foundation;
		needsUpdate = true;
	}
}
