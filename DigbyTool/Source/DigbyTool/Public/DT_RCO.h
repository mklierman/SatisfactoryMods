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
	void Server_MLBMerge(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* groupLeaderOne, ALBBuild_ModularLoadBalancer* groupLeaderTwo);
	FORCEINLINE bool Server_MLBMerge_Validate(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* groupLeaderOne, ALBBuild_ModularLoadBalancer* groupLeaderTwo) { return true; }

	UFUNCTION(Server, BlueprintCallable, WithValidation, Unreliable)
	void Server_MLBSplit(ADT_EquipDigbyTool* DigbyTool, const TArray< ALBBuild_ModularLoadBalancer*>& modulesToBeSplit);
	FORCEINLINE bool Server_MLBSplit_Validate(ADT_EquipDigbyTool* DigbyTool, const TArray< ALBBuild_ModularLoadBalancer*>& modulesToBeSplit) { return true; }

	UFUNCTION(Server, BlueprintCallable, WithValidation, Unreliable)
	void Server_SetFirstGroupLeader(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* module);
	FORCEINLINE bool Server_SetFirstGroupLeader_Validate(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* module) { return true; }

	UFUNCTION(Server, BlueprintCallable, WithValidation, Unreliable)
	void Server_SetSecondGroupLeader(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* module);
	FORCEINLINE bool Server_SetSecondGroupLeader_Validate(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* module) { return true; }

	UFUNCTION(Server, BlueprintCallable, WithValidation, Unreliable)
	void Server_AddModuleToBeSplit(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* module);
	FORCEINLINE bool Server_AddModuleToBeSplit_Validate(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* module) { return true; }

	UFUNCTION(Server, BlueprintCallable, WithValidation, Unreliable)
	void Server_RemoveModuleToBeSplit(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* module);
	FORCEINLINE bool Server_RemoveModuleToBeSplit_Validate(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* module) { return true; }

	UFUNCTION(Server, BlueprintCallable, WithValidation, Unreliable)
	void Server_ResetStuff(ADT_EquipDigbyTool* DigbyTool);
	FORCEINLINE bool Server_ResetStuff_Validate(ADT_EquipDigbyTool* DigbyTool) { return true; }

	UFUNCTION(Server, BlueprintCallable, WithValidation, Unreliable)
	void Server_HighlightGroup(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* groupLeader);
	FORCEINLINE bool Server_HighlightGroup_Validate(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* groupLeader) { return true; }

	UFUNCTION(Server, BlueprintCallable, WithValidation, Unreliable)
	void Server_UnHighlightGroup(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* groupLeader);
	FORCEINLINE bool Server_UnHighlightGroup_Validate(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* groupLeader) { return true; }

	UFUNCTION(Server, BlueprintCallable, WithValidation, Unreliable)
	void Server_UnHighlightAll(ADT_EquipDigbyTool* DigbyTool);
	FORCEINLINE bool Server_UnHighlightAll_Validate(ADT_EquipDigbyTool* DigbyTool) { return true; }

	UFUNCTION(Server, BlueprintCallable, WithValidation, Unreliable)
	void Server_HighlightModule(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* module, UMaterialInterface* holoMaterial);
	FORCEINLINE bool Server_HighlightModule_Validate(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* module, UMaterialInterface* holoMaterial) { return true; }

	UFUNCTION(Server, BlueprintCallable, WithValidation, Unreliable)
	void Server_UnHighlightModule(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* module);
	FORCEINLINE bool Server_UnHighlightModule_Validate(ADT_EquipDigbyTool* DigbyTool, ALBBuild_ModularLoadBalancer* module) { return true; }
};

