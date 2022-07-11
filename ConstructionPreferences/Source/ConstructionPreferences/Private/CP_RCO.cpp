

#include "CP_RCO.h"
#include "FGCharacterPlayer.h"

#include "CP_Subsystem.h"
#include "Subsystem/SubsystemActorManager.h"

void UCP_RCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCP_RCO, bDummy);
}


void UCP_RCO::Server_AddPlayerConfig_Implementation(AFGPlayerController* player, FCP_ModConfigStruct config)
{
	UWorld* WorldObject = player->GetWorld();
	if (WorldObject)
	{
		UE_LOG(LogConstructionPreferences, Display, TEXT("if (WorldObject)"));
		USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
		ACP_Subsystem* mCPSubsystem = SubsystemActorManager->GetSubsystemActor<ACP_Subsystem>();
		if (mCPSubsystem)
		{
			UE_LOG(LogConstructionPreferences, Display, TEXT("if (mCPSubsystem)"));
			mCPSubsystem->PlayerConfigs.Add(player, config);
		}
	}
}

void UCP_RCO::Client_GetPlayerConfig_Implementation()
{

}