#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "FGBuildableSubsystem.h"
#include "Buildables/FGBuildable.h"
#include "FGPlayerController.h"
#include "FGFactoryColoringTypes.h"
#include "FGColorInterface.h"
#include "PersistentPaintablesCppSubSystem.generated.h"


UCLASS()
class APersistentPaintablesCppSubSystem : public AModSubsystem
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable)
		void SetCustomSwatchColor(uint8 ColorSlot, FLinearColor PrimaryColor, FLinearColor SecondaryColor);

	UFUNCTION(BlueprintCallable, Category = "Persistent Paintables")
		void ApplyCustomizationPreview(AFGBuildableSubsystem* buildableSubsystem, AFGBuildable* buildable, const FFactoryCustomizationData& previewData);

	UFUNCTION(BlueprintCallable, Category = "Persistent Paintables")
		TSubclassOf< class UFGItemDescriptor > GetBuildableDescriptor(AFGBuildable* buildable);
};

