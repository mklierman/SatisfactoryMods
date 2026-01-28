

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FGSignTypes.h"
#include "Buildables/FGBuildableWidgetSign.h"
#include "AutSigns_BPFL.generated.h"

/**
 * 
 */
UCLASS()
class AUTOSIGNS_API UAutSigns_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable)
	static void UpdateSignElements(AFGBuildableWidgetSign* sign, FPrefabSignData& prefabSignData);

	UFUNCTION(BlueprintCallable)
	static void SetSignText(AFGBuildableWidgetSign* sign, FString key, FString value);
};
