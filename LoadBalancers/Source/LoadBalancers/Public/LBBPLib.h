#pragma once

#include "CoreMinimal.h"
#include "Resources/FGItemDescriptor.h"
#include "LBBPLib.generated.h"

UCLASS()
class LOADBALANCERS_API ULBBPLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable)
	static void SortItemArray(TArray<TSubclassOf<UFGItemDescriptor>>& Out_Items, const TArray<TSubclassOf<UFGItemDescriptor>>& In_Items, const TArray<TSubclassOf<UFGItemDescriptor>>& mForceFirstItems, bool Reverse = false); 
};
