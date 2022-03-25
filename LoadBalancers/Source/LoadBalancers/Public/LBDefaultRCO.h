#pragma once

#include "FGPlayerController.h"
#include "FGRemoteCallObject.h"
#include "LBBuild_ModularLoadBalancer.h"
#include "LBOutlineSubsystem.h"
#include "LBDefaultRCO.generated.h"

/**
 * 
 */
UCLASS()
class LOADBALANCERS_API ULBDefaultRCO : public UFGRemoteCallObject
{
	GENERATED_BODY()

	virtual void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override;
public:
	UFUNCTION(BlueprintCallable, Category = "Networking", meta = (WorldContext = "WorldContext"))
	static ULBDefaultRCO* GetFFDefaultRCO(UObject* WorldContext) { return Get(WorldContext); }
	static ULBDefaultRCO* Get(UObject* WorldContext)
	{
		if(WorldContext)
			if(AFGPlayerController* Controller = Cast<AFGPlayerController>(WorldContext->GetWorld()->GetFirstPlayerController()))
				if(ULBDefaultRCO* RCO = Controller->GetRemoteCallObjectOfClass<ULBDefaultRCO>())
					return RCO;
		return nullptr;
	};

	UFUNCTION(Server, BlueprintCallable, WithValidation, Unreliable)
	void Server_SetFilteredItem(ALBBuild_ModularLoadBalancer* Balancer, TSubclassOf<UFGItemDescriptor> ItemClass);
	FORCEINLINE bool Server_SetFilteredItem_Validate(ALBBuild_ModularLoadBalancer* Balancer, TSubclassOf<UFGItemDescriptor> ItemClass) { return true; }

	UFUNCTION(Server, BlueprintCallable, WithValidation, Unreliable)
	void Server_CreateOutlineForActor(ALBOutlineSubsystem* Subsystem, AActor* Actor);
	FORCEINLINE bool Server_CreateOutlineForActor_Validate(ALBOutlineSubsystem* Subsystem, AActor* Actor) { return true; }

	UFUNCTION(Server, BlueprintCallable, WithValidation, Unreliable)
	void Server_ClearOutlines(ALBOutlineSubsystem* Subsystem);
	FORCEINLINE bool Server_ClearOutlines_Validate(ALBOutlineSubsystem* Subsystem) { return true; }
	
	UFUNCTION(Server, BlueprintCallable, WithValidation, Unreliable)
	void Server_SetOutlineColor(ALBOutlineSubsystem* Subsystem, FLinearColor Color);
	FORCEINLINE bool Server_SetOutlineColor_Validate(ALBOutlineSubsystem* Subsystem, FLinearColor Color) { return true; }
	
	UFUNCTION(Server, BlueprintCallable, WithValidation, Unreliable)
	void Server_ResetOutlineColor(ALBOutlineSubsystem* Subsystem);
	FORCEINLINE bool Server_ResetOutlineColor_Validate(ALBOutlineSubsystem* Subsystem) { return true; }
	
	UPROPERTY(Replicated)
	bool bDummy = true;
};