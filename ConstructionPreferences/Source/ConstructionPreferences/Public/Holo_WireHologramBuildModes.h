#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hologram/FGWireHologram.h"
#include "FGRecipeManager.h"
#include "Holo_WireHologramBuildModes.generated.h"

/**
 * 
 */
UCLASS()
class CONSTRUCTIONPREFERENCES_API AHolo_WireHologramBuildModes : public AFGWireHologram
{
	GENERATED_BODY()

public:
	AHolo_WireHologramBuildModes();

	virtual void BeginPlay() override;
	virtual void OnBuildModeChanged() override;
	virtual void GetSupportedBuildModes_Implementation(TArray<TSubclassOf<UFGHologramBuildModeDescriptor>>& out_buildmodes) const override;

	void SwitchPoleType();

	UPROPERTY(EditDefaultsOnly, Category = "Construction Preferences")
		TArray<TSubclassOf<UFGRecipe>> PowerPoleRecipes;

	UPROPERTY(EditDefaultsOnly, Category = "Construction Preferences")
		TSubclassOf<UFGRecipe> DefaultPowerPoleRecipe;

	UPROPERTY(EditDefaultsOnly, Category = "Construction Preferences")
		TArray<TSubclassOf<UFGRecipe>> WallSocketRecipes;

	UPROPERTY(EditDefaultsOnly, Category = "Construction Preferences")
		TSubclassOf<UFGRecipe> DefaultWallSocketRecipe;

	UPROPERTY(EditDefaultsOnly, Category = "Construction Preferences")
		TArray<TSubclassOf<UFGHologramBuildModeDescriptor>> PoleBuildModes;

	UPROPERTY(EditDefaultsOnly, Category = "Construction Preferences")
		TArray<TSubclassOf<UFGHologramBuildModeDescriptor>> WallBuildModes;

	void GetRecipeManager();

	AFGRecipeManager* RecipeManager;
	bool AllowSetActiveHologram;
};
