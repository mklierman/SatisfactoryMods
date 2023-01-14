#pragma once

#include "FGRemoteCallObject.h"
#include "FGPlayerController.h"
#include "PP_RCO.generated.h"

UCLASS()
class PERSISTENTPAINTABLES_API UPP_RCO : public UFGRemoteCallObject
{
	GENERATED_BODY()

	virtual void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override;

public:

	UFUNCTION(BlueprintCallable, Category = "Networking", meta = (WorldContext = "WorldContext"))
	static UPP_RCO* GetPersistentPaintablesDefaultRCO(UObject* WorldContext) { return Get(WorldContext); }

	static UPP_RCO* Get(UObject* WorldContext)
	{
		if(WorldContext)
		{
			if(AFGPlayerController* Controller = Cast<AFGPlayerController>(WorldContext->GetWorld()->GetFirstPlayerController()))
			{
				if(UPP_RCO* RCO = Controller->GetRemoteCallObjectOfClass<UPP_RCO>())
				{
					return RCO;
				}
			}
		}
		return nullptr;
	};

	 UFUNCTION(Server, BlueprintCallable, WithValidation, Unreliable)
	 void Server_SetSavedConfig();
	 FORCEINLINE bool Server_SetSavedConfig_Validate() { return true; }
	
	// This is needed for GetLifetimeReplicatedProps (I think.. maybe it isn't anymore)
	UPROPERTY(Replicated)
	bool bDummy = true;
};