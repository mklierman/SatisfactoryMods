

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PipeTuner_BPFL.generated.h"

/**
 * 
 */
UCLASS()
class PIPETUNER_API UPipeTuner_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable)
	static void UpdateValues(UGameInstance* instance);
};
