

#pragma once

#include "CoreMinimal.h"
#include "FGPlayerController.h"
#include "FGRemoteCallObject.h"
#include "HPPR_RCO.generated.h"

/**
 *
 */
UCLASS()
class HOVERPACKPOLERANGE_API UHPPR_RCO : public UFGRemoteCallObject
{
	GENERATED_BODY()

		virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:

	UFUNCTION(BlueprintCallable, Category = "Networking", meta = (WorldContext = "WorldContext"))
	static UHPPR_RCO* GetFFDefaultRCO(UObject* WorldContext) { return Get(WorldContext); }
	static UHPPR_RCO* Get(UObject* WorldContext)
	{
		if (WorldContext)
			if (AFGPlayerController* Controller = Cast<AFGPlayerController>(WorldContext->GetWorld()->GetFirstPlayerController()))
				if (UHPPR_RCO* RCO = Controller->GetRemoteCallObjectOfClass<UHPPR_RCO>())
					return RCO;
		return nullptr;
	};

	UPROPERTY(Replicated)
		bool bDummy = true;
};
