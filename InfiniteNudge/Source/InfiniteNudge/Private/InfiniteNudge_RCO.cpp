#include "InfiniteNudge_RCO.h"
//DEFINE_LOG_CATEGORY(InfiniteNudge_Log);

void UInfiniteNudge_RCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInfiniteNudge_RCO, bDummy);
}

void UInfiniteNudge_RCO::Server_SetHologramScale_Implementation(AFGPlayerController* controller, FVector newScale)
{
	if (controller)
	{
		auto player = Cast<AFGCharacterPlayer>(controller->GetControlledCharacter());
		if (player)
		{
			auto buildState = Cast<UFGBuildGunStateBuild>(player->GetBuildGun()->GetCurrentState());
			if (buildState)
			{
				auto currentHolo = buildState->GetHologram();
				if (currentHolo)
				{
					currentHolo->SetActorRelativeScale3D(newScale);
					currentHolo->ForceNetUpdate();
				}
			}
		}
	}
	//if (hologram)
	//{
	//	UE_LOG(InfiniteNudge_Log, Display, TEXT("Server_SetHologramScale - NewScale: %s"), *newScale.ToString());
	//	hologram->SetActorRelativeScale3D(newScale);
	//	hologram->ForceNetUpdate();
	//}
}
