#pragma once

#include "CoreMinimal.h"
#include "HypertubeDestinationTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HypertubeRoutePlanner.generated.h"

class AFGCharacterPlayer;
class AFGPipeHyperStart;

UCLASS()
class HYPERTUBEDESTINATIONS_API UHypertubeRoutePlanner : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Finds the shortest route by number of hypertube pieces. Must run on the server. */
	UFUNCTION(BlueprintCallable, Category = "Hypertube Destinations|Routing")
	static bool FindRoute(AFGCharacterPlayer* Player, AFGPipeHyperStart* Source, AFGPipeHyperStart* Destination, FHypertubeRoutePlan& OutRoute);
};
