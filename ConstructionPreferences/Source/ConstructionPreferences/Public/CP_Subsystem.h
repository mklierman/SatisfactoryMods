

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "FGSaveInterface.h"
#include "Hologram/FGHologramBuildModeDescriptor.h"
#include "FGRecipe.h"
#include "CP_Subsystem.generated.h"

USTRUCT(BlueprintType, Category = "Construction Preferences")
struct FWireBuildModeStruct
{
	GENERATED_BODY()

		UPROPERTY(SaveGame, EditDefaultsOnly, Category = "Construction Preferences")
		TSubclassOf<UFGHologramBuildModeDescriptor> BuildMode;

	UPROPERTY(SaveGame, EditDefaultsOnly, Category = "Construction Preferences")
		TSubclassOf<UFGRecipe> Recipe;

	UPROPERTY(SaveGame, EditDefaultsOnly, Category = "Construction Preferences")
		bool IsWallSocket;
	UPROPERTY(SaveGame, EditDefaultsOnly, Category = "Construction Preferences")
		bool IsDoubleWallSocket;
};
/**
 *
 */
UCLASS()
class CONSTRUCTIONPREFERENCES_API ACP_Subsystem : public AModSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()
public:
	ACP_Subsystem();

	virtual bool ShouldSave_Implementation() const override;

	UPROPERTY(SaveGame)
	FWireBuildModeStruct LastUsedPoleBuildMode;

	UPROPERTY(SaveGame)
	FWireBuildModeStruct LastUsedWallSockedBuildMode;
};
