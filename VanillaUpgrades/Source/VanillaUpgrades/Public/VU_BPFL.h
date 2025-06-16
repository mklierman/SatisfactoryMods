

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Buildables/FGBuildablePowerStorage.h"
#include "Buildables/FGBuildableResourceExtractor.h"
#include "Buildables/FGBuildableGeneratorFuel.h"
#include "Buildables/FGBuildableFactory.h"
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
	
	UFUNCTION(BlueprintCallable)
	static void FuelGenBeginPlay(AFGBuildableGeneratorFuel* fuelGen);

	UFUNCTION(BlueprintCallable)
	static void FactoryBeginPlay(AFGBuildableFactory* factory);

	UFUNCTION(BlueprintCallable)
	static void StartPowerProduction(AFGBuildableGenerator* generator);
};
