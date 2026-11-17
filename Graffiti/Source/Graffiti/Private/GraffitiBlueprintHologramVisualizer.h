#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Hologram/FGHologram.h"
#include "GraffitiBlueprintHologramVisualizer.generated.h"

class AFGBuildableGraffitiContainer;
class AFGBlueprintHologram;
class UDecalComponent;
class UMaterialInstanceDynamic;
class USceneComponent;

UCLASS()
class UGraffitiBlueprintHologramVisualizer : public UActorComponent
{
	GENERATED_BODY()

public:
	UGraffitiBlueprintHologramVisualizer();

	void Initialize(
		AFGBlueprintHologram* BlueprintHologram,
		const AFGBuildableGraffitiContainer* Container,
		USceneComponent* BuildableRoot);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void ApplyHologramTint(bool bForce);
	FLinearColor ResolveHologramTint(EHologramMaterialState State) const;

	UPROPERTY(Transient)
	TObjectPtr<AFGBlueprintHologram> Hologram;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UDecalComponent>> PreviewDecals;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> PreviewMaterials;

	EHologramMaterialState LastMaterialState = static_cast<EHologramMaterialState>(MAX_uint8);
};

