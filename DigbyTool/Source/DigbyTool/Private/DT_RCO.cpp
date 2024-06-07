#include "DT_RCO.h"
#include <Net/UnrealNetwork.h>

void UDT_RCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UDT_RCO, bDummy);


}

void UDT_RCO::Server_RagdollPlayer_Implementation(AFGCharacterBase* Character)
{
	Character->RagdollCharacter(true);
}