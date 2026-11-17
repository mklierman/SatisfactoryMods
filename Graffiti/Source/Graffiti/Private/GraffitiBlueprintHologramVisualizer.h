#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GraffitiBlueprintHologramVisualizer.generated.h"

class AFGBuildableGraffitiContainer;
class AFGBlueprintHologram;
class UDecalComponent;
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

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UDecalComponent>> PreviewDecals;
};

