#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "FGBuildableSubsystem.h"
#include "Buildables/FGBuildable.h"
#include "FGPlayerController.h"
#include "FGFactoryColoringTypes.h"
#include "FGColorInterface.h"
#include "PersistentPaintablesCppSubSystem.generated.h"

USTRUCT(BlueprintType)
struct PERSISTENTPAINTABLES_API FPlayerCustomizationStruct
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	AFGCharacterPlayer* CharacterPlayer;
	UPROPERTY(BlueprintReadWrite)
	AFGPlayerController* PlayerController;
	UPROPERTY(BlueprintReadWrite)
	FFactoryCustomizationData CustomizationData;
};

UCLASS()
class APersistentPaintablesCppSubSystem : public AModSubsystem
{
	GENERATED_BODY()
public:
	APersistentPaintablesCppSubSystem();

	UFUNCTION(BlueprintCallable)
		void HookConstruct();


	UFUNCTION(BlueprintCallable)
		void PaintBuildableConstructed(AFGBuildable* buildable);

	UFUNCTION(BlueprintCallable)
		void SetCustomSwatchColor(uint8 ColorSlot, FLinearColor PrimaryColor, FLinearColor SecondaryColor);

	UFUNCTION(BlueprintCallable, Category = "Persistent Paintables")
		void ApplyCustomizationPreview(AFGBuildableSubsystem* buildableSubsystem, AFGBuildable* buildable, const FFactoryCustomizationData& previewData);

	UFUNCTION(BlueprintCallable, Category = "Persistent Paintables")
		TSubclassOf< class UFGItemDescriptor > GetBuildableDescriptor(AFGBuildable* buildable);

	UPROPERTY(BlueprintReadWrite)
	TArray<FPlayerCustomizationStruct> PlayerCustomizationStructs;
};

