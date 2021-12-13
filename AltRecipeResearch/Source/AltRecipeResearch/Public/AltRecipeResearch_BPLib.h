
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AltRecipeResearch_BPLib.generated.h"

UCLASS()
class ALTRECIPERESEARCH_API UAltRecipeResearch_BPLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

		UFUNCTION(BlueprintCallable, Category = "AltRecipeResearch")
        static void PopulateResearchTrees(UWorld* World);
    };