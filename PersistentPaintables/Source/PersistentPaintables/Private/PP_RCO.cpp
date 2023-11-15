#include "PP_RCO.h"
#include <Net/UnrealNetwork.h>

void UPP_RCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UPP_RCO, bDummy);
}

 void UPP_RCO::Server_SetSavedConfig_Implementation()
 {
 }
