#include "GraffitiPlacementComponent.h"

#include "Components/DecalComponent.h"
#include "FGIconDatabaseSubsystem.h"
#include "FGPlayerController.h"
#include "GameFramework/Pawn.h"
#include "GraffitiConstants.h"
#include "GraffitiIconUtils.h"
#include "GraffitiPlacementMath.h"
#include "GraffitiRemoteCallObject.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

UGraffitiPlacementComponent::UGraffitiPlacementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UGraffitiPlacementComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
}

void UGraffitiPlacementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyPreviewComponent();
	Super::EndPlay(EndPlayReason);
}

void UGraffitiPlacementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!bPlacementActive || !PreviewDecal)
	{
		return;
	}

	APlayerController* Controller = ResolvePlayerController();
	if (!Controller || !GetWorld())
	{
		SetHasValidSurface(false);
		PreviewDecal->SetVisibility(false);
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GraffitiPreview), true, Controller->GetPawn());
	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		ViewLocation,
		ViewLocation + ViewRotation.Vector() * FMath::Clamp(MaximumDistance, Graffiti::MinDistance, Graffiti::MaxDistance),
		ECC_Visibility,
		QueryParams);

	SetHasValidSurface(bHit);
	PreviewDecal->SetVisibility(bHit);
	if (!bHit)
	{
		return;
	}

	const FVector PlacementPoint = bGridSnapEnabled
		? GraffitiPlacementMath::SnapPointToWorldGrid(Hit.ImpactPoint, Hit.ImpactNormal, FMath::Clamp(GridSnapSize, Graffiti::MinGridSize, Graffiti::MaxGridSize))
		: Hit.ImpactPoint;
	PreviewDecal->SetWorldLocation(PlacementPoint + Hit.ImpactNormal * Graffiti::SurfaceOffset);
	PreviewDecal->SetWorldRotation(GraffitiPlacementMath::MakeDecalRotation(Hit.ImpactNormal, RollDegrees));
	PreviewDecal->DecalSize = FVector(Graffiti::DecalDepth, PreviewSize * 0.5f, PreviewSize * 0.5f);
	PreviewDecal->MarkRenderTransformDirty();
	PreviewDecal->MarkRenderStateDirty();
}

bool UGraffitiPlacementComponent::StartPlacement(const FPersistentGlobalIconId& IconId, FLinearColor Tint, float InitialSize, float InitialRollDegrees)
{
	if (!Graffiti::ResolveIconTexture(this, IconId))
	{
		return false;
	}

	SelectedIconId = IconId;
	SelectedTint = Tint.GetClamped();
	PreviewSize = FMath::Clamp(InitialSize, Graffiti::MinSize, Graffiti::MaxSize);
	RollDegrees = FMath::UnwindDegrees(InitialRollDegrees);
	bMirrorHorizontal = false;
	bGridSnapEnabled = false;
	bPlacementActive = true;
	SetHasValidSurface(false);

	UpdatePreviewMaterial();
	EnsurePreviewComponent();
	if (!PreviewDecal || !DynamicMaterial)
	{
		CancelPlacement();
		return false;
	}
	SetComponentTickEnabled(true);
	return true;
}

bool UGraffitiPlacementComponent::StartPlacementFromLocalIconId(int32 LocalIconId, FLinearColor Tint, float InitialSize, float InitialRollDegrees)
{
	AFGIconDatabaseSubsystem* Database = AFGIconDatabaseSubsystem::Get(this);
	if (!Database || !Database->IsInitialized())
	{
		return false;
	}
	return StartPlacement(Database->ResolveLocalIconId(LocalIconId), Tint, InitialSize, InitialRollDegrees);
}

bool UGraffitiPlacementComponent::ConfirmPlacement()
{
	if (!bPlacementActive || !bHasValidSurface)
	{
		return false;
	}

	if (UGraffitiRemoteCallObject* RCO = UGraffitiRemoteCallObject::Get(this))
	{
		RCO->ServerSpray(
			SelectedIconId,
			SelectedTint,
			PreviewSize,
			RollDegrees,
			FMath::Clamp(MaximumDistance, Graffiti::MinDistance, Graffiti::MaxDistance),
			bMirrorHorizontal,
			bGridSnapEnabled,
			FMath::Clamp(GridSnapSize, Graffiti::MinGridSize, Graffiti::MaxGridSize));
		CancelPlacement();
		return true;
	}
	return false;
}

void UGraffitiPlacementComponent::CancelPlacement()
{
	bPlacementActive = false;
	SetHasValidSurface(false);
	SetComponentTickEnabled(false);
	if (PreviewDecal)
	{
		PreviewDecal->SetVisibility(false);
	}
}

void UGraffitiPlacementComponent::AdjustRotation(float DeltaDegrees)
{
	if (bPlacementActive && FMath::IsFinite(DeltaDegrees))
	{
		RollDegrees = FMath::UnwindDegrees(RollDegrees + DeltaDegrees);
	}
}

void UGraffitiPlacementComponent::AdjustScale(float DeltaSize)
{
	if (bPlacementActive && FMath::IsFinite(DeltaSize))
	{
		SetPlacementScale(PreviewSize + DeltaSize);
	}
}

void UGraffitiPlacementComponent::SetPlacementScale(float NewSize)
{
	if (FMath::IsFinite(NewSize))
	{
		PreviewSize = FMath::Clamp(NewSize, Graffiti::MinSize, Graffiti::MaxSize);
	}
}

void UGraffitiPlacementComponent::ToggleMirrorHorizontal()
{
	if (bPlacementActive)
	{
		SetMirrorHorizontal(!bMirrorHorizontal);
	}
}

void UGraffitiPlacementComponent::SetMirrorHorizontal(bool bMirrored)
{
	bMirrorHorizontal = bMirrored;
	if (DynamicMaterial)
	{
		DynamicMaterial->SetScalarParameterValue(Graffiti::MirrorParameter, bMirrorHorizontal ? 1.0f : 0.0f);
	}
}

void UGraffitiPlacementComponent::SetGridSnappingEnabled(bool bEnabled)
{
	bGridSnapEnabled = bEnabled;
}

void UGraffitiPlacementComponent::ToggleGridSnapping()
{
	bGridSnapEnabled = !bGridSnapEnabled;
}

void UGraffitiPlacementComponent::SetGridSnapSize(float NewGridSize)
{
	if (FMath::IsFinite(NewGridSize))
	{
		GridSnapSize = FMath::Clamp(NewGridSize, Graffiti::MinGridSize, Graffiti::MaxGridSize);
	}
}

APlayerController* UGraffitiPlacementComponent::ResolvePlayerController() const
{
	if (APlayerController* Controller = Cast<APlayerController>(GetOwner()))
	{
		return Controller;
	}
	if (const APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		return Cast<APlayerController>(Pawn->GetController());
	}
	return GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
}

void UGraffitiPlacementComponent::EnsurePreviewComponent()
{
	if (PreviewDecal || !GetWorld() || !DynamicMaterial)
	{
		return;
	}

	PreviewDecal = UGameplayStatics::SpawnDecalAtLocation(
		GetWorld(), DynamicMaterial, FVector(Graffiti::DecalDepth, PreviewSize * 0.5f, PreviewSize * 0.5f),
		FVector::ZeroVector, FRotator::ZeroRotator, 0.0f);
	if (!PreviewDecal)
	{
		return;
	}
	PreviewDecal->SetVisibility(false);
	PreviewDecal->FadeScreenSize = Graffiti::PreviewFadeScreenSize;
	PreviewDecal->SortOrder = Graffiti::PreviewSortOrder;
}

void UGraffitiPlacementComponent::DestroyPreviewComponent()
{
	if (PreviewDecal)
	{
		PreviewDecal->DestroyComponent();
		PreviewDecal = nullptr;
		DynamicMaterial = nullptr;
	}
}

void UGraffitiPlacementComponent::UpdatePreviewMaterial()
{
	if (!DecalMaterial)
	{
		DecalMaterial = LoadObject<UMaterialInterface>(nullptr, Graffiti::DecalMaterialPath);
	}
	if (!DecalMaterial)
	{
		return;
	}

	UTexture* IconTexture = Graffiti::ResolveIconTexture(this, SelectedIconId);
	if (!IconTexture)
	{
		return;
	}

	DynamicMaterial = UMaterialInstanceDynamic::Create(DecalMaterial, this);
	DynamicMaterial->SetTextureParameterValue(Graffiti::IconTextureParameter, IconTexture);
	DynamicMaterial->SetVectorParameterValue(Graffiti::TintParameter, SelectedTint);
	DynamicMaterial->SetScalarParameterValue(Graffiti::MirrorParameter, bMirrorHorizontal ? 1.0f : 0.0f);
	if (PreviewDecal)
	{
		PreviewDecal->SetDecalMaterial(DynamicMaterial);
	}
}

void UGraffitiPlacementComponent::SetHasValidSurface(bool bNewValue)
{
	if (bHasValidSurface != bNewValue)
	{
		bHasValidSurface = bNewValue;
		OnPlacementValidityChanged.Broadcast(bHasValidSurface);
	}
}
