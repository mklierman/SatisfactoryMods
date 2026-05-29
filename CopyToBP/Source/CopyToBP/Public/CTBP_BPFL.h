

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CTBP_BPFL.generated.h"

/**
 * 
 */
UCLASS()
class COPYTOBP_API UCTBP_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable)
	static void GetAllSubclassesOf(
		TSubclassOf<UObject> ParentClass,
		TArray<UClass*>& OutClassess
	);

	UFUNCTION(BlueprintCallable)
	static void SetShouldPasteInConfig(bool shouldPaste, UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable)
	static void SetConfigValue(bool newValue, UConfigProperty* property);
};
