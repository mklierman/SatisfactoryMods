#pragma once
#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "FGPlayerController.h"
#include "Hologram/FGHologram.h"
#include "FGCharacterPlayer.h"
#include "Equipment/FGBuildGun.h"
#include "Equipment/FGBuildGunBuild.h"
#include <Net/UnrealNetwork.h>
#include "InfiniteNudge_RCO.generated.h"

UCLASS()
class UInfiniteNudge_RCO : public UFGRemoteCallObject
{
	GENERATED_BODY()
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	static UInfiniteNudge_RCO* GetFFDefaultRCO(UObject* WorldContext) { return Get(WorldContext); }
	static UInfiniteNudge_RCO* Get(UObject* WorldContext)
	{
		if (WorldContext)
		{
			if (AFGPlayerController* Controller = Cast<AFGPlayerController>(WorldContext->GetWorld()->GetFirstPlayerController()))
			{
				if (UInfiniteNudge_RCO* RCO = Controller->GetRemoteCallObjectOfClass<UInfiniteNudge_RCO>())
				{
					return RCO;
				}
			}
		}
		return nullptr;
	};

	UPROPERTY(Replicated)
	bool bDummy = true;

	UFUNCTION(Server, BlueprintCallable, WithValidation, Unreliable)
	void Server_SetHologramScale(AFGPlayerController* controller, FVector newScale);
	FORCEINLINE bool Server_SetHologramScale_Validate(AFGPlayerController* controller, FVector newScale) { return true; }
};

