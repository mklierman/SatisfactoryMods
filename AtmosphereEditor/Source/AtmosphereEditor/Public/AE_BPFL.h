#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Interfaces/Interface_PostProcessVolume.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveLinearColor.h"
#include "GameFramework/Volume.h"
#include "FGSkySphere.h"
#include "FGAtmosphereVolume.h"
#include "Atmosphere/UFGBiome.h"
#include "Atmosphere/BiomeHelpers.h"
#include "Interfaces/Interface_PostProcessVolume.h"
#include "FGSkySphere.h"
#include "GameFramework/Volume.h"

#include "AE_BPFL.generated.h"

UCLASS()
class ATMOSPHEREEDITOR_API UAE_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	public:
	UFUNCTION(BlueprintCallable)
	static TArray<UFGBiome*> GetAllBiomes();

	UFUNCTION(BlueprintCallable)
	static UFGBiome* GetGrassyFieldsMain();
	//UFUNCTION(BlueprintCallable)
	//static FExponentialFogSettings GetFogSettings(AFGAtmosphereVolume* volume);
	//UFUNCTION(BlueprintCallable)
	//static FSkySphereSettings GetSkySphereSettings(AFGAtmosphereVolume* volume, float atTime);
	//UFUNCTION(BlueprintCallable)
	//static FSkyAtmosphereSettings GetAtmosphereSettings(AFGAtmosphereVolume* volume, float atTime);

	////UFUNCTION(BlueprintCallable)
	////static FPostProcessVolumeProperties GetPostProcessProperties(AFGAtmosphereVolume* volume);

	//UFUNCTION(BlueprintCallable)
	//static void GetBiomeExponentialFogSettings(UFGBiome* biome, float atTime, struct FExponentialFogSettings& out_settings);

	//UFUNCTION(BlueprintCallable)
	//static void GetBiomeSkySphereSettings(UFGBiome* biome, float atTime, struct FSkySphereSettings& out_settings);

	//UFUNCTION(BlueprintCallable)
	//static void GetBiomeAtmosphereSettings(UFGBiome* biome, float atTime, struct FSkyAtmosphereSettings& out_settings);
};
