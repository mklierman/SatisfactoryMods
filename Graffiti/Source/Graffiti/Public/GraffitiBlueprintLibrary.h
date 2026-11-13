#pragma once

#include "CoreMinimal.h"
#include "FGIconLibrary.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GraffitiBlueprintLibrary.generated.h"

class UGraffitiPlacementComponent;

UCLASS()
class GRAFFITI_API UGraffitiBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Graffiti|Icons", meta = (WorldContext = "WorldContext"))
	static void GetAvailableGraffitiIcons(UObject* WorldContext, bool IncludeHidden, TArray<FIconData>& OutIcons);

	UFUNCTION(BlueprintPure, Category = "Graffiti|Icons", meta = (WorldContext = "WorldContext"))
	static FPersistentGlobalIconId MakePersistentGraffitiIconId(UObject* WorldContext, const FIconData& IconData);

	UFUNCTION(BlueprintPure, Category = "Graffiti|Icons", meta = (WorldContext = "WorldContext"))
	static FPersistentGlobalIconId MakePersistentGraffitiIconIdFromLocalId(UObject* WorldContext, int32 LocalIconId);

	UFUNCTION(BlueprintPure, Category = "Graffiti|Icons", meta = (WorldContext = "WorldContext"))
	static FPersistentGlobalIconId MakePersistentGraffitiIconIdFromPicker(UObject* WorldContext, int32 PickerIconId, UObject* PickerTexture);

	UFUNCTION(BlueprintPure, Category = "Graffiti|Icons", meta = (WorldContext = "WorldContext"))
	static UObject* ResolveGraffitiIconTexture(UObject* WorldContext, const FPersistentGlobalIconId& IconId);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Icons|Testing", meta = (WorldContext = "WorldContext"))
	static bool GetGraffitiIconById(UObject* WorldContext, int32 IconId, FIconData& OutIconData, FPersistentGlobalIconId& OutPersistentIconId);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Icons|Testing", meta = (WorldContext = "WorldContext"))
	static bool GetRandomGraffitiIcon(UObject* WorldContext, bool IncludeHidden, FIconData& OutIconData, FPersistentGlobalIconId& OutPersistentIconId);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Spray", meta = (WorldContext = "WorldContext"))
	static bool RequestSprayGraffiti(
		UObject* WorldContext,
		const FPersistentGlobalIconId& IconId,
		FLinearColor Tint,
		float Size = 200.0f,
		float RollDegrees = 0.0f,
		float MaximumDistance = 1500.0f,
		bool MirrorHorizontal = false,
		bool SnapToGrid = false,
		float GridSize = 100.0f);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Spray", meta = (WorldContext = "WorldContext"))
	static bool RequestEraseGraffiti(UObject* WorldContext, float MaximumDistance = 1500.0f);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement", meta = (WorldContext = "WorldContext"))
	static bool StartGraffitiPlacement(UObject* WorldContext, const FPersistentGlobalIconId& IconId, FLinearColor Tint, float InitialSize = 200.0f, float InitialRollDegrees = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement", meta = (WorldContext = "WorldContext"))
	static bool StartGraffitiPlacementFromLocalIconId(UObject* WorldContext, int32 LocalIconId, FLinearColor Tint, float InitialSize = 200.0f, float InitialRollDegrees = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement", meta = (WorldContext = "WorldContext"))
	static bool StartGraffitiPlacementFromPicker(UObject* WorldContext, int32 PickerIconId, UObject* PickerTexture, FLinearColor Tint, float InitialSize = 200.0f, float InitialRollDegrees = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement", meta = (WorldContext = "WorldContext"))
	static bool ToggleGraffitiPlacementFromLocalIconId(UObject* WorldContext, int32 LocalIconId, FLinearColor Tint, float InitialSize = 200.0f, float InitialRollDegrees = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement", meta = (WorldContext = "WorldContext"))
	static bool ToggleGraffitiPlacementFromPicker(UObject* WorldContext, int32 PickerIconId, UObject* PickerTexture, FLinearColor Tint, float InitialSize = 200.0f, float InitialRollDegrees = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement", meta = (WorldContext = "WorldContext"))
	static bool ConfirmGraffitiPlacement(UObject* WorldContext);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement", meta = (WorldContext = "WorldContext"))
	static void CancelGraffitiPlacement(UObject* WorldContext);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement", meta = (WorldContext = "WorldContext"))
	static void AdjustGraffitiPlacementRotation(UObject* WorldContext, float DeltaDegrees);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement", meta = (WorldContext = "WorldContext"))
	static void AdjustGraffitiPlacementScale(UObject* WorldContext, float DeltaSize);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement", meta = (WorldContext = "WorldContext"))
	static void SetGraffitiPlacementScale(UObject* WorldContext, float NewSize);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement", meta = (WorldContext = "WorldContext"))
	static void ToggleGraffitiPlacementMirror(UObject* WorldContext);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement", meta = (WorldContext = "WorldContext"))
	static void SetGraffitiPlacementMirrored(UObject* WorldContext, bool Mirrored);

	UFUNCTION(BlueprintPure, Category = "Graffiti|Placement", meta = (WorldContext = "WorldContext"))
	static bool IsGraffitiPlacementMirrored(UObject* WorldContext);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement", meta = (WorldContext = "WorldContext"))
	static void SetGraffitiPlacementGridSnapping(UObject* WorldContext, bool Enabled);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement", meta = (WorldContext = "WorldContext"))
	static void ToggleGraffitiPlacementGridSnapping(UObject* WorldContext);

	UFUNCTION(BlueprintCallable, Category = "Graffiti|Placement", meta = (WorldContext = "WorldContext"))
	static void SetGraffitiPlacementGridSize(UObject* WorldContext, float GridSize);

	UFUNCTION(BlueprintPure, Category = "Graffiti|Placement", meta = (WorldContext = "WorldContext"))
	static bool IsGraffitiPlacementGridSnapping(UObject* WorldContext);

	UFUNCTION(BlueprintPure, Category = "Graffiti|Placement", meta = (WorldContext = "WorldContext"))
	static bool IsGraffitiPlacementActive(UObject* WorldContext);

	UFUNCTION(BlueprintPure, Category = "Graffiti|Placement", meta = (WorldContext = "WorldContext"))
	static bool HasValidGraffitiPlacementSurface(UObject* WorldContext);

private:
	static UGraffitiPlacementComponent* GetPlacementComponent(UObject* WorldContext, bool bCreateIfMissing);
};
