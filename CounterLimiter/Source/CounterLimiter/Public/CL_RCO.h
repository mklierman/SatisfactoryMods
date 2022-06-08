

#pragma once

#include "CoreMinimal.h"
#include "FGPlayerController.h"
#include "FGRemoteCallObject.h"
#include "CL_CounterLimiter.h"
#include "CL_RCO.generated.h"

/**
 *
 */
UCLASS()
class COUNTERLIMITER_API UCL_RCO : public UFGRemoteCallObject
{
	GENERATED_BODY()

		virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:
	UFUNCTION(BlueprintCallable, Category = "Networking", meta = (WorldContext = "WorldContext"))
	static UCL_RCO* GetFFDefaultRCO(UObject* WorldContext) { return Get(WorldContext); }
	static UCL_RCO* Get(UObject* WorldContext)
	{
		UE_LOG(CounterLimiter_Log, Display, TEXT("Get RCO"));
		if (WorldContext)
		{
			UE_LOG(CounterLimiter_Log, Display, TEXT("Has WorldContext"));
			if (AFGPlayerController* Controller = Cast<AFGPlayerController>(WorldContext->GetWorld()->GetFirstPlayerController()))
			{
				UE_LOG(CounterLimiter_Log, Display, TEXT("Has Controller"));
				if (UCL_RCO* RCO = Controller->GetRemoteCallObjectOfClass<UCL_RCO>())
				{
					UE_LOG(CounterLimiter_Log, Display, TEXT("Has RCO"));
					return RCO;
				}
			}
		}
		return nullptr;
	};

	UPROPERTY(Replicated)
	bool bDummy = true;

	UFUNCTION(Server, BlueprintCallable, WithValidation, Unreliable)
	void Server_SetLimiterRate(ACL_CounterLimiter* Limiter, float newRate);
	FORCEINLINE bool Server_SetLimiterRate_Validate(ACL_CounterLimiter* Limiter, float newRate) { return true; }

	UFUNCTION(Server, BlueprintCallable, WithValidation, Unreliable)
	void Server_CalculateIPM(ACL_CounterLimiter* Limiter);
	FORCEINLINE bool Server_CalculateIPM_Validate(ACL_CounterLimiter* Limiter) { return true; }
};
