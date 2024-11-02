// ILikeBanas

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/SpotLightComponent.h"
#include "FlashlightSettings_ModConfigStruct.h"
#include "FlashlightSettingsBPFunctionLib.generated.h"

UCLASS()
class FLASHLIGHTSETTINGS_API UFlashlightSettingsBPFunctionLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "FlashlightSettings")
	static void StoreFlashlightSettings(UConfigPropertySection* Section, USpotLightComponent* SpotLight);

	UFUNCTION(BlueprintCallable, Category = "FlashlightSettings", BlueprintPure = false, meta = (WorldContext = "WorldContext"))
	static FFlashlightSettings_ModConfigStruct_SavedSettings GetFlashlightSettings(UObject* WorldContext);

	UFUNCTION(BlueprintCallable, Category = "FlashlightSettings")
	static void ApplyFlashlightSettings(const FFlashlightSettings_ModConfigStruct_SavedSettings& SavedSettings, USpotLightComponent* SpotLight);
};
