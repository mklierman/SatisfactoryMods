

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include <FGCharacterMovementComponent.h>
#include "HyperTubeConfigurator_BPFL.generated.h"

/**
 * 
 */
UCLASS()
class HYPERTUBECONFIGURATOR_API UHyperTubeConfigurator_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	static void SetHypertubeStuff(UFGCharacterMovementComponent* movementComp);
	
	
};
