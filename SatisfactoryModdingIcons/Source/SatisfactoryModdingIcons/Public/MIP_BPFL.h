

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MIP_BPFL.generated.h"

/**
 * 
 */
UCLASS()
class SATISFACTORYMODDINGICONS_API UMIP_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	
	UFUNCTION(BlueprintCallable)
	static void RegisterIcons();
	
};
