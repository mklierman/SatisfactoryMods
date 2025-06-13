

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "FGItemPickup_Spawnable.h"
#include "OutlineLootSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogOutlineLoot, Display, All);
/**
 * 
 */
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
	void ApplySingle(AFGItemPickup_Spawnable* pickup);

	UFUNCTION()
	void OnColorChanged();

	UFUNCTION()
	void OnSizeChanged();

	UMaterialInstance* material;

	FDelegateHandle Hook;
	FScriptDelegate colorChanged;
	FScriptDelegate sizeChanged;
};
