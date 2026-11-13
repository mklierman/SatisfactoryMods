#include "GraffitiBlueprintLibrary.h"

#include "FGIconDatabaseSubsystem.h"
#include "FGPlayerController.h"
#include "Engine/Texture.h"
#include "GraffitiIconUtils.h"
#include "GraffitiPlacementComponent.h"
#include "GraffitiRemoteCallObject.h"

void UGraffitiBlueprintLibrary::GetAvailableGraffitiIcons(UObject* WorldContext, bool IncludeHidden, TArray<FIconData>& OutIcons)
{
	OutIcons.Reset();
	AFGIconDatabaseSubsystem* Database = AFGIconDatabaseSubsystem::Get(WorldContext);
	if (!Database || !Database->IsInitialized())
	{
		return;
	}

	TArray<FIconData> AllIcons;
	Database->GetIconData(AllIcons);
	for (const FIconData& Icon : AllIcons)
	{
		if ((!Icon.Hidden || IncludeHidden) && Cast<UTexture>(UFGIconLibrary::GetTextureFromIconData(Icon)))
		{
			OutIcons.Add(Icon);
		}
	}
}

FPersistentGlobalIconId UGraffitiBlueprintLibrary::MakePersistentGraffitiIconId(UObject* WorldContext, const FIconData& IconData)
{
	return MakePersistentGraffitiIconIdFromLocalId(WorldContext, IconData.ID);
}

FPersistentGlobalIconId UGraffitiBlueprintLibrary::MakePersistentGraffitiIconIdFromLocalId(UObject* WorldContext, int32 LocalIconId)
{
	if (AFGIconDatabaseSubsystem* Database = AFGIconDatabaseSubsystem::Get(WorldContext); Database && Database->IsInitialized())
	{
		return Database->ResolveLocalIconId(LocalIconId);
	}
	return FPersistentGlobalIconId{};
}

FPersistentGlobalIconId UGraffitiBlueprintLibrary::MakePersistentGraffitiIconIdFromPicker(UObject* WorldContext, int32 PickerIconId, UObject* PickerTexture)
{
	AFGIconDatabaseSubsystem* Database = AFGIconDatabaseSubsystem::Get(WorldContext);
	if (!Database || !Database->IsInitialized())
	{
		return FPersistentGlobalIconId{};
	}

	const int32 TextureIconId = PickerTexture ? Database->GetIconIDForTexture(PickerTexture) : INDEX_NONE;
	const int32 LocalIconId = TextureIconId != INDEX_NONE ? TextureIconId : PickerIconId;
	FIconData IconData;
	if (LocalIconId == INDEX_NONE || !Database->GetIconDataForIconID(LocalIconId, IconData))
	{
		return FPersistentGlobalIconId{};
	}
	return Database->ResolveLocalIconId(LocalIconId);
}

UObject* UGraffitiBlueprintLibrary::ResolveGraffitiIconTexture(UObject* WorldContext, const FPersistentGlobalIconId& IconId)
{
	return Graffiti::ResolveIconTexture(WorldContext, IconId);
}

bool UGraffitiBlueprintLibrary::GetGraffitiIconById(UObject* WorldContext, int32 IconId, FIconData& OutIconData, FPersistentGlobalIconId& OutPersistentIconId)
{
	OutIconData = FIconData{};
	OutPersistentIconId = FPersistentGlobalIconId{};

	AFGIconDatabaseSubsystem* Database = AFGIconDatabaseSubsystem::Get(WorldContext);
	if (!Database || !Database->IsInitialized() || !Database->GetIconDataForIconID(IconId, OutIconData))
	{
		return false;
	}

	if (!Cast<UTexture>(UFGIconLibrary::GetTextureFromIconData(OutIconData)))
	{
		OutIconData = FIconData{};
		return false;
	}

	OutPersistentIconId = Database->ResolveLocalIconId(IconId);
	return OutPersistentIconId.IconID != INDEX_NONE;
}

bool UGraffitiBlueprintLibrary::GetRandomGraffitiIcon(UObject* WorldContext, bool IncludeHidden, FIconData& OutIconData, FPersistentGlobalIconId& OutPersistentIconId)
{
	OutIconData = FIconData{};
	OutPersistentIconId = FPersistentGlobalIconId{};

	TArray<FIconData> AvailableIcons;
	GetAvailableGraffitiIcons(WorldContext, IncludeHidden, AvailableIcons);
	if (AvailableIcons.IsEmpty())
	{
		return false;
	}

	OutIconData = AvailableIcons[FMath::RandHelper(AvailableIcons.Num())];
	OutPersistentIconId = MakePersistentGraffitiIconId(WorldContext, OutIconData);
	return OutPersistentIconId.IconID != INDEX_NONE;
}

bool UGraffitiBlueprintLibrary::RequestSprayGraffiti(
	UObject* WorldContext,
	const FPersistentGlobalIconId& IconId,
	FLinearColor Tint,
	float Size,
	float RollDegrees,
	float MaximumDistance,
	bool MirrorHorizontal,
	bool SnapToGrid,
	float GridSize)
{
	if (UGraffitiRemoteCallObject* RCO = UGraffitiRemoteCallObject::Get(WorldContext))
	{
		RCO->ServerSpray(IconId, Tint, Size, RollDegrees, MaximumDistance, MirrorHorizontal, SnapToGrid, GridSize);
		return true;
	}
	return false;
}

bool UGraffitiBlueprintLibrary::RequestEraseGraffiti(UObject* WorldContext, float MaximumDistance)
{
	if (UGraffitiRemoteCallObject* RCO = UGraffitiRemoteCallObject::Get(WorldContext))
	{
		RCO->ServerErase(MaximumDistance);
		return true;
	}
	return false;
}

bool UGraffitiBlueprintLibrary::StartGraffitiPlacement(UObject* WorldContext, const FPersistentGlobalIconId& IconId, FLinearColor Tint, float InitialSize, float InitialRollDegrees)
{
	if (UGraffitiPlacementComponent* Placement = GetPlacementComponent(WorldContext, true))
	{
		return Placement->StartPlacement(IconId, Tint, InitialSize, InitialRollDegrees);
	}
	return false;
}

bool UGraffitiBlueprintLibrary::StartGraffitiPlacementFromLocalIconId(UObject* WorldContext, int32 LocalIconId, FLinearColor Tint, float InitialSize, float InitialRollDegrees)
{
	if (UGraffitiPlacementComponent* Placement = GetPlacementComponent(WorldContext, true))
	{
		return Placement->StartPlacementFromLocalIconId(LocalIconId, Tint, InitialSize, InitialRollDegrees);
	}
	return false;
}

bool UGraffitiBlueprintLibrary::StartGraffitiPlacementFromPicker(UObject* WorldContext, int32 PickerIconId, UObject* PickerTexture, FLinearColor Tint, float InitialSize, float InitialRollDegrees)
{
	const FPersistentGlobalIconId IconId = MakePersistentGraffitiIconIdFromPicker(WorldContext, PickerIconId, PickerTexture);
	return IconId.IconID != INDEX_NONE && StartGraffitiPlacement(WorldContext, IconId, Tint, InitialSize, InitialRollDegrees);
}

bool UGraffitiBlueprintLibrary::ToggleGraffitiPlacementFromLocalIconId(UObject* WorldContext, int32 LocalIconId, FLinearColor Tint, float InitialSize, float InitialRollDegrees)
{
	UGraffitiPlacementComponent* Placement = GetPlacementComponent(WorldContext, true);
	if (!Placement)
	{
		return false;
	}
	return Placement->IsPlacementActive()
		? Placement->ConfirmPlacement()
		: Placement->StartPlacementFromLocalIconId(LocalIconId, Tint, InitialSize, InitialRollDegrees);
}

bool UGraffitiBlueprintLibrary::ToggleGraffitiPlacementFromPicker(UObject* WorldContext, int32 PickerIconId, UObject* PickerTexture, FLinearColor Tint, float InitialSize, float InitialRollDegrees)
{
	UGraffitiPlacementComponent* Placement = GetPlacementComponent(WorldContext, true);
	if (!Placement)
	{
		return false;
	}
	if (Placement->IsPlacementActive())
	{
		return Placement->ConfirmPlacement();
	}

	const FPersistentGlobalIconId IconId = MakePersistentGraffitiIconIdFromPicker(WorldContext, PickerIconId, PickerTexture);
	return IconId.IconID != INDEX_NONE && Placement->StartPlacement(IconId, Tint, InitialSize, InitialRollDegrees);
}

bool UGraffitiBlueprintLibrary::ConfirmGraffitiPlacement(UObject* WorldContext)
{
	if (UGraffitiPlacementComponent* Placement = GetPlacementComponent(WorldContext, false))
	{
		return Placement->ConfirmPlacement();
	}
	return false;
}

void UGraffitiBlueprintLibrary::CancelGraffitiPlacement(UObject* WorldContext)
{
	if (UGraffitiPlacementComponent* Placement = GetPlacementComponent(WorldContext, false))
	{
		Placement->CancelPlacement();
	}
}

void UGraffitiBlueprintLibrary::AdjustGraffitiPlacementRotation(UObject* WorldContext, float DeltaDegrees)
{
	if (UGraffitiPlacementComponent* Placement = GetPlacementComponent(WorldContext, false))
	{
		Placement->AdjustRotation(DeltaDegrees);
	}
}

void UGraffitiBlueprintLibrary::AdjustGraffitiPlacementScale(UObject* WorldContext, float DeltaSize)
{
	if (UGraffitiPlacementComponent* Placement = GetPlacementComponent(WorldContext, false))
	{
		Placement->AdjustScale(DeltaSize);
	}
}

void UGraffitiBlueprintLibrary::SetGraffitiPlacementScale(UObject* WorldContext, float NewSize)
{
	if (UGraffitiPlacementComponent* Placement = GetPlacementComponent(WorldContext, false))
	{
		Placement->SetPlacementScale(NewSize);
	}
}

void UGraffitiBlueprintLibrary::ToggleGraffitiPlacementMirror(UObject* WorldContext)
{
	if (UGraffitiPlacementComponent* Placement = GetPlacementComponent(WorldContext, false))
	{
		Placement->ToggleMirrorHorizontal();
	}
}

void UGraffitiBlueprintLibrary::SetGraffitiPlacementMirrored(UObject* WorldContext, bool Mirrored)
{
	if (UGraffitiPlacementComponent* Placement = GetPlacementComponent(WorldContext, false))
	{
		Placement->SetMirrorHorizontal(Mirrored);
	}
}

bool UGraffitiBlueprintLibrary::IsGraffitiPlacementMirrored(UObject* WorldContext)
{
	if (const UGraffitiPlacementComponent* Placement = GetPlacementComponent(WorldContext, false))
	{
		return Placement->IsPlacementMirroredHorizontally();
	}
	return false;
}

void UGraffitiBlueprintLibrary::SetGraffitiPlacementGridSnapping(UObject* WorldContext, bool Enabled)
{
	if (UGraffitiPlacementComponent* Placement = GetPlacementComponent(WorldContext, false))
	{
		Placement->SetGridSnappingEnabled(Enabled);
	}
}

void UGraffitiBlueprintLibrary::ToggleGraffitiPlacementGridSnapping(UObject* WorldContext)
{
	if (UGraffitiPlacementComponent* Placement = GetPlacementComponent(WorldContext, false))
	{
		Placement->ToggleGridSnapping();
	}
}

void UGraffitiBlueprintLibrary::SetGraffitiPlacementGridSize(UObject* WorldContext, float GridSize)
{
	if (UGraffitiPlacementComponent* Placement = GetPlacementComponent(WorldContext, true))
	{
		Placement->SetGridSnapSize(GridSize);
	}
}

float UGraffitiBlueprintLibrary::GetGraffitiPlacementGridSize(UObject* WorldContext)
{
	if (const UGraffitiPlacementComponent* Placement = GetPlacementComponent(WorldContext, false))
	{
		return Placement->GetGridSnapSize();
	}
	return 100.0f;
}

bool UGraffitiBlueprintLibrary::IsGraffitiPlacementGridSnapping(UObject* WorldContext)
{
	if (const UGraffitiPlacementComponent* Placement = GetPlacementComponent(WorldContext, false))
	{
		return Placement->IsGridSnappingEnabled();
	}
	return false;
}

bool UGraffitiBlueprintLibrary::IsGraffitiPlacementActive(UObject* WorldContext)
{
	if (const UGraffitiPlacementComponent* Placement = GetPlacementComponent(WorldContext, false))
	{
		return Placement->IsPlacementActive();
	}
	return false;
}

bool UGraffitiBlueprintLibrary::HasValidGraffitiPlacementSurface(UObject* WorldContext)
{
	if (const UGraffitiPlacementComponent* Placement = GetPlacementComponent(WorldContext, false))
	{
		return Placement->HasValidPlacementSurface();
	}
	return false;
}

UGraffitiPlacementComponent* UGraffitiBlueprintLibrary::GetPlacementComponent(UObject* WorldContext, bool bCreateIfMissing)
{
	if (!WorldContext || !WorldContext->GetWorld())
	{
		return nullptr;
	}

	AFGPlayerController* Controller = Cast<AFGPlayerController>(WorldContext->GetWorld()->GetFirstPlayerController());
	if (!Controller)
	{
		return nullptr;
	}

	if (UGraffitiPlacementComponent* Existing = Controller->FindComponentByClass<UGraffitiPlacementComponent>())
	{
		return Existing;
	}
	if (!bCreateIfMissing)
	{
		return nullptr;
	}

	UGraffitiPlacementComponent* Placement = NewObject<UGraffitiPlacementComponent>(Controller, TEXT("GraffitiPlacementComponent"));
	Controller->AddInstanceComponent(Placement);
	Placement->RegisterComponent();
	return Placement;
}
