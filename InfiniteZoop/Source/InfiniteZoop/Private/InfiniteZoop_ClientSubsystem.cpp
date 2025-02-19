#include "InfiniteZoop_ClientSubsystem.h"
#include "Subsystem/SubsystemActorManager.h"
#include <Logging/StructuredLog.h>

AInfiniteZoop_ClientSubsystem::AInfiniteZoop_ClientSubsystem()
{
	ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnClient;
}
AInfiniteZoop_ClientSubsystem* AInfiniteZoop_ClientSubsystem::Get(UObject* worldContext)
{
	if (worldContext)
	{
		UWorld* WorldObject = GEngine->GetWorldFromContextObjectChecked(worldContext);
		USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
		check(SubsystemActorManager);
		return Cast<AInfiniteZoop_ClientSubsystem>(SubsystemActorManager->K2_GetSubsystemActor(StaticClass()));
	}
	return nullptr;
}

void AInfiniteZoop_ClientSubsystem::SetPublicZoopAmount(int x, int y, int z, bool foundation, bool verticalZoop)
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
//
//bool AInfiniteZoop_ClientSubsystem::ShouldSave_Implementation() const
//{
//    return true;
//}