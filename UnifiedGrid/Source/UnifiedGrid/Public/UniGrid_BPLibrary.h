// ILikeBanas

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include <future>
#include "FactoryGame/Public/Hologram/FGHologram.h"
#include "FactoryGame/Public/Buildables/FGBuildable.h"
#include "FactoryGame/Public/Resources/FGResourceNodeBase.h"
#include "UniGrid_BPLibrary.generated.h"

/**
 * 
 */

UCLASS()
class UNIFIEDGRID_API UUniGrid_BPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		UFUNCTION(BlueprintCallable, Category = "UniGrid")
		static AActor* SpawnAndBuildFromRecipe(AFGBuildableSubsystem* buildableSubsystem, TSubclassOf<class UFGRecipe> inRecipe, AActor* hologramOwner,FTransform nodeTransform, FVector nodeLocation, AActor* primaryBuilding, AActor* node, float alignmentHeight, float extraHeight);
};