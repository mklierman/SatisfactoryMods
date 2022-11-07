#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hologram/FGWireHologram.h"
#include "FGRecipeManager.h"
#include "CP_Subsystem.h"
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
		TSubclassOf<UFGRecipe> DefaultPowerPoleRecipe;

	UPROPERTY(EditDefaultsOnly, Category = "Construction Preferences")
		TSubclassOf<UFGRecipe> DefaultWallSocketRecipe;

	UPROPERTY(EditDefaultsOnly, Category = "Construction Preferences")
		TArray<FWireBuildModeStruct> WireBuildModes;

	void GetRecipeManager();

	AFGRecipeManager* RecipeManager;

private:
	ACP_Subsystem* CP_Subsystem;
	APawn* ConstructionInstigator;

	void RespawnChildPoleHolograms();
	void RespawnChildWallSocketHolograms();
	void SetSafeDefaultWallSocket();
};
