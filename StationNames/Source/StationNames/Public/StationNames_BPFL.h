

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Buildables/FGBuildableDockingStation.h"
#include "FGMapAreaTexture.h"
#include "FGMapArea.h"
#include "StationNames_BPFL.generated.h"

/**
 *
 */
UCLASS()
class STATIONNAMES_API UStationNames_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

		UFUNCTION(BlueprintPure, Category = "Station Names")
		static void AutoSetDockingStationName(AFGBuildableDockingStation* DockingStation, FString& OutMapArea, FString& OutLoadMode, FString& OutItemName);

		UFUNCTION(BlueprintPure, Category = "Station Names")
			static FString GetRandomizedName();

		UFUNCTION(BlueprintPure, Category = "Station Names")
			static TSubclassOf< UFGMapArea > GetMapAreaForWorldLocation(UFGMapAreaTexture* mapAreaTexture, FVector worldLocation);
};
