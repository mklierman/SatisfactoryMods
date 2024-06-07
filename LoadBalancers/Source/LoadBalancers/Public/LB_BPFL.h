

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LBBuild_ModularLoadBalancer.h"
#include "LB_BPFL.generated.h"

/**
 *
 */
UCLASS()
class LOADBALANCERS_API ULB_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
		UFUNCTION(BlueprintCallable, Category = "Utilities")
		static void SortStringArray(UPARAM(ref)TArray<FString>& Array_To_Sort, TArray<FString>& Sorted_Array);


		UFUNCTION(BlueprintCallable, Category = "MLB")
		static void MergeBalancerGroups(ALBBuild_ModularLoadBalancer* groupLeaderOne, ALBBuild_ModularLoadBalancer* groupLeaderTwo);


		UFUNCTION(BlueprintCallable, Category = "MLB")
		static void SplitBalancers(TArray< ALBBuild_ModularLoadBalancer*> modulesToBeSplit);
};
