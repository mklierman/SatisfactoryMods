#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Buildables/FGBuildableConveyorBelt.h"
#include "GAFB_BPFL.generated.h"

struct FGAFBLinkedBeltNetwork
{
	TArray<AFGBuildableConveyorBelt*> Belts;
	TArray<AFGBuildableConveyorAttachment*> Attachments;
};

/**
 * 
 */
UCLASS()
class GRABALLFROMBELT_API UGAFB_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	static void ClearBelt(AFGBuildableConveyorBelt* belt, AFGPlayerController* controller);

	static void ClearBeltVisual(AFGBuildableConveyorBelt* belt);
	
	UFUNCTION(BlueprintCallable)
	static void ClearLinkedBelts(AFGBuildableConveyorBelt* belt, AFGPlayerController* controller);

	static void ClearLinkedBeltsAndCollect(AFGBuildableConveyorBelt* belt, AFGPlayerController* controller,	TArray<AFGBuildableConveyorBelt*>& outBelts);

	UFUNCTION(BlueprintCallable)
	static void ClearLinkedBeltsRemote(AFGBuildableConveyorBelt* belt, AFGPlayerController* controller);

	UFUNCTION(BlueprintCallable)
	static void ClearSingleBeltRemote(AFGBuildableConveyorBelt* belt, AFGPlayerController* controller);

private:
	static void RemoveAllItemsFromChain(AFGConveyorChainActor* chain);

	static void ClearBeltChain(AFGBuildableConveyorBelt* belt, AFGPlayerController* controller, bool giveItems);

	static void GatherLinkedBeltNetworkRecursive(AFGBuildableConveyorBelt* belt, TSet<AFGBuildableConveyorBelt*>& visitedBelts,	TSet<AFGBuildableConveyorAttachment*>& visitedAttachments, FGAFBLinkedBeltNetwork& outNetwork);

	static FGAFBLinkedBeltNetwork GatherLinkedBeltNetwork(AFGBuildableConveyorBelt* belt);
};
