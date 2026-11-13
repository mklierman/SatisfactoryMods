#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FGIconLibrary.h"
#include "GraffitiPlacementComponent.generated.h"

class APlayerController;
class UDecalComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGraffitiPlacementValidityChanged, bool, bHasValidSurface);

UCLASS(ClassGroup = (Graffiti), meta = (BlueprintSpawnableComponent))
class GRAFFITI_API UGraffitiPlacementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGraffitiPlacementComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement")
	bool StartPlacement(const FPersistentGlobalIconId& IconId, FLinearColor Tint, float InitialSize = 200.0f, float InitialRollDegrees = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement")
	bool StartPlacementFromLocalIconId(int32 LocalIconId, FLinearColor Tint, float InitialSize = 200.0f, float InitialRollDegrees = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement")
	bool ConfirmPlacement();

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement")
	void CancelPlacement();

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement")
	void AdjustRotation(float DeltaDegrees);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement")
	void AdjustScale(float DeltaSize);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement")
	void SetPlacementScale(float NewSize);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement")
	void ToggleMirrorHorizontal();

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement")
	void SetMirrorHorizontal(bool bMirrored);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement")
	void SetGridSnappingEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement")
	void ToggleGridSnapping();

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement")
	void SetGridSnapSize(float NewGridSize);

	UFUNCTION(BlueprintPure, Category = "Graffiti|Placement")
	bool IsPlacementActive() const { return bPlacementActive; }

	UFUNCTION(BlueprintPure, Category = "Graffiti|Placement")
	bool HasValidPlacementSurface() const { return bHasValidSurface; }

	UFUNCTION(BlueprintPure, Category = "Graffiti|Placement")
	float GetPlacementRotation() const { return RollDegrees; }

	UFUNCTION(BlueprintPure, Category = "Graffiti|Placement")
	float GetPlacementScale() const { return PreviewSize; }

	UFUNCTION(BlueprintPure, Category = "Graffiti|Placement")
	bool IsPlacementMirroredHorizontally() const { return bMirrorHorizontal; }

	UFUNCTION(BlueprintPure, Category = "Graffiti|Placement")
	bool IsGridSnappingEnabled() const { return bGridSnapEnabled; }

	UFUNCTION(BlueprintPure, Category = "Graffiti|Placement")
	float GetGridSnapSize() const { return GridSnapSize; }

	UPROPERTY(BlueprintAssignable, Category = "Graffiti|Placement")
	FGraffitiPlacementValidityChanged OnPlacementValidityChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graffiti|Placement", meta = (ClampMin = "100.0", ClampMax = "3000.0"))
	float MaximumDistance = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graffiti|Placement", meta = (ClampMin = "1.0"))
	float RotationStep = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graffiti|Placement", meta = (ClampMin = "1.0"))
	float ScaleStep = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graffiti|Placement", meta = (ClampMin = "1.0", ClampMax = "1000.0"))
	float GridSnapSize = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graffiti|Placement")
	FLinearColor InvalidPreviewTint = FLinearColor(1.0f, 0.1f, 0.1f, 0.45f);

private:
	APlayerController* ResolvePlayerController() const;
	void EnsurePreviewComponent();
	void DestroyPreviewComponent();
	void UpdatePreviewMaterial();
	void SetHasValidSurface(bool bNewValue);

	UPROPERTY(Transient)
	TObjectPtr<UDecalComponent> PreviewDecal;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> DecalMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	UPROPERTY(Transient)
	FPersistentGlobalIconId SelectedIconId;

	UPROPERTY(Transient)
	FLinearColor SelectedTint = FLinearColor::White;

	UPROPERTY(Transient)
	float PreviewSize = 200.0f;

	UPROPERTY(Transient)
	float RollDegrees = 0.0f;

	UPROPERTY(Transient)
	bool bMirrorHorizontal = false;

	UPROPERTY(Transient)
	bool bGridSnapEnabled = false;

	UPROPERTY(Transient)
	bool bPlacementActive = false;

	UPROPERTY(Transient)
	bool bHasValidSurface = false;
};
