

#pragma once

#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "FGPlayerController.h"
#include "CP_ModConfigStruct.h"
#include "CP_RCO.generated.h"

/**
 *
 */
UCLASS()
class CONSTRUCTIONPREFERENCES_API UCP_RCO : public UFGRemoteCallObject
{
	GENERATED_BODY()

		virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:
	UFUNCTION(BlueprintCallable, Category = "Networking", meta = (WorldContext = "WorldContext"))
		static UCP_RCO* GetFFDefaultRCO(UObject* WorldContext) { return Get(WorldContext); }
	static UCP_RCO* Get(UObject* WorldContext)
	{
		if (WorldContext)
		{
			if (AFGPlayerController* Controller = Cast<AFGPlayerController>(WorldContext->GetWorld()->GetFirstPlayerController()))
			{
				if (UCP_RCO* RCO = Controller->GetRemoteCallObjectOfClass<UCP_RCO>())
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
		void Server_AddPlayerConfig(AFGPlayerController* player, FCP_ModConfigStruct config);
		FORCEINLINE bool Server_AddPlayerConfig_Validate(AFGPlayerController* player, FCP_ModConfigStruct config) { return true; }


		UFUNCTION(Client, BlueprintCallable, WithValidation, Unreliable)
		void Client_GetPlayerConfig();
		FORCEINLINE bool Client_GetPlayerConfig_Validate() { return true; }
};
