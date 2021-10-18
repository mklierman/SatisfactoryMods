// ILikeBanas

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HologramLocation_BPLib.generated.h"

/**
 * 
 */
UCLASS()
class HOLOGRAMLOCATION_API UHologramLocation_BPLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

		UFUNCTION(BlueprintCallable, Category = "HologramLocation")
		static FString RegexReplace(FString inputString, FString regexCode, FString replacementText);

	UFUNCTION(BlueprintCallable, Category = "HologramLocation")
		static FString AddCommasToInt(int inputInt);
};
