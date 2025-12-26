

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/SCS_Node.h"
#include "FGAttachmentPointComponent.h"
#include "FGAttachmentPoint.h"
#include "MSS_BPFL.generated.h"

/**
 * 
 */
UCLASS()
class MORESIGNSPOTS_API UMSS_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable, Category = "MoreSignSpots")
	static void AddSignSpotToDecorator(UClass* DecoClass, FName Name, FTransform Transform, EAttachmentPointUsage Usage, TSubclassOf< class UFGAttachmentPointType > Type);
};
