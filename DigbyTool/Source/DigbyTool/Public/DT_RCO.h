#pragma once

#include "CoreMinimal.h"
#include "FGPlayerController.h"
#include "FGRemoteCallObject.h"
#include "DT_EquipDigbyTool.h"
#include "DT_RCO.generated.h"

/**
 *
 */
UCLASS()

class UDT_RCO :  public UFGRemoteCallObject
{
	GENERATED_BODY()
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION(BlueprintCallable, Category = "Networking", meta = (WorldContext = "WorldContext"))
	static UDT_RCO* GetDTDefaultRCO(UObject* WorldContext) { return Get(WorldContext); }
	static UDT_RCO* Get(UObject* WorldContext)
	{
		if (WorldContext)
		{
			if (AFGPlayerController* Controller = Cast<AFGPlayerController>(WorldContext->GetWorld()->GetFirstPlayerController()))
			{
				if (UDT_RCO* RCO = Controller->GetRemoteCallObjectOfClass<UDT_RCO>())
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
	void Server_RagdollPlayer(AFGCharacterBase* Character);
	FORCEINLINE bool Server_RagdollPlayer_Validate(AFGCharacterBase* Character) { return true; }


};

