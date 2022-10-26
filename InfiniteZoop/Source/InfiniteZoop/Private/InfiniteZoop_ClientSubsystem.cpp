#include "InfiniteZoop_ClientSubsystem.h"
#include "Subsystem/SubsystemActorManager.h"

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
//
//bool AInfiniteZoop_ClientSubsystem::ShouldSave_Implementation() const
//{
//    return true;
//}