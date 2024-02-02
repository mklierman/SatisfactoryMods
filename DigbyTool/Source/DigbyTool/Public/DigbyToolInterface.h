#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FGColoredInstanceMeshProxy.h"
#include "Equipment/FGEquipment.h"
#include "LBBuild_ModularLoadBalancer.h"
#include "DT_EquipDigbyTool.h"
#include "DigbyToolInterface.generated.h"

UINTERFACE(Blueprintable)
class UDigbyToolInterface :  public UInterface
{
	GENERATED_BODY()
};

class IDigbyToolInterface
{
	GENERATED_BODY()

public:
	EToolMode currentToolMode;

	EToolError currentToolError;

	ALBBuild_ModularLoadBalancer* firstGroupLeader;

	ALBBuild_ModularLoadBalancer* secondGroupLeader;

	TArray< ALBBuild_ModularLoadBalancer*> splitBalancers;

	TArray< ALBBuild_ModularLoadBalancer*> balancersToSplit;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Digby Tool")
	void SetFirstGroupLeader(ALBBuild_ModularLoadBalancer* balancer);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Digby Tool")
	void SetSecondGroupLeader(ALBBuild_ModularLoadBalancer* balancer);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Digby Tool")
	void MergeGroups();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Digby Tool")
	void SplitGroups();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Digby Tool")
	EToolMode GetCurrentToolMode();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Digby Tool")
	void SetCurrentToolMode(EToolMode newMode);
};

