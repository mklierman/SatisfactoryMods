

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Smokey_BPFL.generated.h"

class AFGBuildable;
class AFGBuildableFactory;

UCLASS()
class SMOKEY_API USmokey_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void SetGeneratorSignificanceRange(AFGBuildableFactory* generator, float newRange);

	UFUNCTION(BlueprintCallable)
	static void ConfigureBuildableParticleSystems(AFGBuildable* buildable);

	UFUNCTION(BlueprintCallable)
	static void RestoreBuildableParticleSystems(AFGBuildable* buildable);
};
