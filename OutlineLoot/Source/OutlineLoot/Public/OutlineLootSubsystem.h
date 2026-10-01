#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "FGItemPickup_Spawnable.h"
#include "OutlineLootSubsystem.generated.h"

UCLASS()
class OUTLINELOOT_API AOutlineLootSubsystem : public AModSubsystem
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	FLinearColor GetColor(AActor* actor);
	float GetScale(AActor* actor);
	void ApplyToAll();
	void ApplySingle(AFGItemPickup* pickup);

	UFUNCTION()
	void OnColorChanged();

	UFUNCTION()
	void OnSizeChanged();

	UMaterialInstance* material;

	FDelegateHandle Hook;
	FDelegateHandle ItemPickupHook;
	FDelegateHandle VisualsHook;
	FScriptDelegate colorChanged;
	FScriptDelegate sizeChanged;

	TSet<TWeakObjectPtr<AFGItemPickup>> InstancedFruitPickups;
};
