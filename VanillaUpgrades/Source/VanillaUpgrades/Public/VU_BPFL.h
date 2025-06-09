

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Buildables/FGBuildablePowerStorage.h"
#include "Buildables/FGBuildableResourceExtractor.h"
#include "VU_BPFL.generated.h"

/**
 * 
 */
UCLASS()
class VANILLAUPGRADES_API UVU_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	static void PowerStorageBeginPlay(AFGBuildablePowerStorage* powerStorage);

	UFUNCTION(BlueprintCallable)
	static void CalculateProductionCycleTime(AFGBuildableResourceExtractor* extractor);
};
