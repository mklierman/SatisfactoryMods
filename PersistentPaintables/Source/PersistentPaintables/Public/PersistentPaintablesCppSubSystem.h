#pragma once

#include "AbstractInstanceManager.h"
#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildablePipeline.h"
#include "Buildables/FGBuildablePipelineJunction.h"
#include "CoreMinimal.h"
//#include "FGBuildablePipelineSupport.h"
#include "FGBuildableSubsystem.h"
#include "FGColorInterface.h"
#include "FGFactoryColoringTypes.h"
#include "FGFluidIntegrantInterface.h"
#include "FGPipeConnectionComponent.h"
#include "FGPipeNetwork.h"
#include "FGPlayerController.h"
#include "Kismet\KismetSystemLibrary.h"
#include "Resources/FGItemDescriptor.h"
#include "Subsystem/ModSubsystem.h"
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

	void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;



	UFUNCTION(BlueprintCallable)
		void HookConstruct();

	UFUNCTION(BlueprintCallable)
		void HookPipeNetwork();

	UPROPERTY(EditDefaultsOnly)
	UClass* wallSupportClass;
	UPROPERTY(EditDefaultsOnly)
	UClass* wallHoleClass;
	UPROPERTY(EditDefaultsOnly)
	UClass* floorHoleClass;
	UPROPERTY(EditDefaultsOnly)
	UClass* swatchClass;

	UFUNCTION(BlueprintCallable)
	void UpdateColor(AFGPipeNetwork* pipeNetwork);

	void UpdateColorSingle(AFGBuildable* buildable, AFGPipeNetwork* pipeNetwork);

	void ApplyColor(AFGBuildable* buildable, UClass* inSwatchClass, FFactoryCustomizationData customizationData);


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

	void AddBuildable(class AFGBuildable* buildable);

	FDelegateHandle ConstructHook;
};

