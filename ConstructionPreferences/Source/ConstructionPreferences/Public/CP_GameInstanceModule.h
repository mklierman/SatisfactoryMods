#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildablePoleStackable.h"
#include "ConstructionPreferencesModule.h"
#include "FGRecipe.h"
#include "Hologram/FGConveyorPoleHologram.h"
#include "Hologram/FGStackablePoleHologram.h"
#include "Module/GameInstanceModule.h"
#include "CP_GameInstanceModule.generated.h"

UCLASS(Abstract)
class CONSTRUCTIONPREFERENCES_API UCP_GameInstanceModule : public UGameInstanceModule
{
	GENERATED_BODY()
public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif

	static UCP_GameInstanceModule* Get(UObject* WorldContext);

	void ScanOneRecipe(TSubclassOf<UFGRecipe> Recipe);

	virtual void DispatchLifecycleEvent(ELifecyclePhase Phase) override;

	UPROPERTY()
	TSet<TSubclassOf<UFGRecipe>> ConveyorBeltRecipes;

	UPROPERTY()
	UObject* StackablePoleCDO;

	UPROPERTY(EditDefaultsOnly, Category = "Construction Preferences")
	TSubclassOf<UFGRecipe> Recipe_ConveyorPoleStackable;

	UPROPERTY(EditDefaultsOnly, Category = "Construction Preferences")
	TSubclassOf<AFGBuildablePoleStackable> Build_ConveyorPoleStackable;

	UPROPERTY(EditDefaultsOnly, Category = "Construction Preferences")
	TSubclassOf<AFGStackablePoleHologram> Holo_ConveyorStackable;

	UPROPERTY(EditDefaultsOnly, Category = "Construction Preferences")
	TSubclassOf<AFGConveyorPoleHologram> Holo_CP_Stackable_Belt;
};
