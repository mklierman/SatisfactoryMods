// ILikeBanas

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/SpotLightComponent.h"
#include "FlashlightSettingsBPFunctionLib.generated.h"

/**
 * 
 */
UCLASS()
class FLASHLIGHTSETTINGS_API UFlashlightSettingsBPFunctionLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

		UFUNCTION(BlueprintCallable, Category = "FlashlightSettings")
		static bool UseTemperature(USpotLightComponent* SpotLightComp);

		UFUNCTION(BlueprintCallable, Category = "FlashlightSettings")
		void TestAccessors();
};
