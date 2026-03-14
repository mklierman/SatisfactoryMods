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

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static AActor* CloneActor(UObject* WorldContextObject, AActor* ActorToClone, FTransform NewTransform, FName Tag);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static AActor* CloneActorLocal(UObject* WorldContextObject, AActor* ActorToClone, FTransform NewTransform, FName Tag);
};
