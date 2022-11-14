#pragma once

#include "CoreMinimal.h"
#include "Hologram/FGWireHologram.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "CP_BPFL.generated.h"

UCLASS()
class CONSTRUCTIONPREFERENCES_API UCP_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	public:
	UFUNCTION(BlueprintCallable)
	 static TArray<TSubclassOf<class AFGBuildableWire>> GetPowerWireClasses();
};
