#include "GraffitiBlueprintHologramVisualizer.h"

#include "Components/DecalComponent.h"
#include "FGGameUserSettings.h"
#include "FGBuildableGraffitiContainer.h"
#include "GraffitiConstants.h"
#include "GraffitiIconUtils.h"
#include "Hologram/FGBlueprintHologram.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

UGraffitiBlueprintHologramVisualizer::UGraffitiBlueprintHologramVisualizer()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UGraffitiBlueprintHologramVisualizer::Initialize(
	AFGBlueprintHologram* BlueprintHologram,
	const AFGBuildableGraffitiContainer* Container,
	USceneComponent* BuildableRoot)
{
	Hologram = BlueprintHologram;
	if (!IsValid(Hologram) || !IsValid(Container) || !IsValid(BuildableRoot))
	{
		SetComponentTickEnabled(false);
		return;
	}

	UMaterialInterface* DecalMaterial = LoadObject<UMaterialInterface>(nullptr, Graffiti::DecalMaterialPath);
	if (!DecalMaterial)
	{
		SetComponentTickEnabled(false);
		return;
	}

	for (const FGraffitiContainerEntry& Entry : Container->GetEntries())
	{
		UTexture* IconTexture = Graffiti::ResolveIconTexture(Hologram, Entry.IconId);
		if (!IconTexture)
		{
			continue;
		}

		UDecalComponent* Decal = UGameplayStatics::SpawnDecalAttached(
			DecalMaterial,
			FVector(Graffiti::DecalDepth, Entry.Size * 0.5f, Entry.Size * 0.5f),
			BuildableRoot,
			NAME_None,
			Entry.RelativeTransform.GetLocation(),
			Entry.RelativeTransform.Rotator(),
			EAttachLocation::KeepRelativeOffset,
			0.0f);
		if (!Decal)
		{
			continue;
		}

		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(DecalMaterial, Decal);
		DynamicMaterial->SetTextureParameterValue(Graffiti::IconTextureParameter, IconTexture);
		DynamicMaterial->SetScalarParameterValue(Graffiti::MirrorParameter, Entry.bMirrorHorizontal ? 1.0f : 0.0f);
		Decal->SetDecalMaterial(DynamicMaterial);
		Decal->FadeScreenSize = Graffiti::PreviewFadeScreenSize;
		Decal->SetSortOrder(Entry.LayerOrder);
		PreviewDecals.Add(Decal);
		PreviewMaterials.Add(DynamicMaterial);
	}

	ApplyHologramTint(true);
}

void UGraffitiBlueprintHologramVisualizer::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ApplyHologramTint(false);
}

void UGraffitiBlueprintHologramVisualizer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (UDecalComponent* Decal : PreviewDecals)
	{
		if (IsValid(Decal))
		{
			Decal->DestroyComponent();
		}
	}
	PreviewDecals.Reset();
	PreviewMaterials.Reset();
	Super::EndPlay(EndPlayReason);
}

void UGraffitiBlueprintHologramVisualizer::ApplyHologramTint(bool bForce)
{
	if (!IsValid(Hologram))
	{
		return;
	}

	const EHologramMaterialState State = Hologram->GetHologramMaterialState();
	if (!bForce && State == LastMaterialState)
	{
		return;
	}

	LastMaterialState = State;
	const FLinearColor Tint = ResolveHologramTint(State);
	for (UMaterialInstanceDynamic* Material : PreviewMaterials)
	{
		if (IsValid(Material))
		{
			Material->SetVectorParameterValue(Graffiti::TintParameter, Tint);
		}
	}
}

FLinearColor UGraffitiBlueprintHologramVisualizer::ResolveHologramTint(EHologramMaterialState State) const
{
	const UFGGameUserSettings* Settings = UFGGameUserSettings::GetFGGameUserSettings();
	FVector Color(0.0f, 0.75f, 1.0f);
	if (Settings)
	{
		switch (State)
		{
		case EHologramMaterialState::HMS_WARNING:
			Color = Settings->GetHologramSoftClearanceColor();
			break;
		case EHologramMaterialState::HMS_ERROR:
			Color = Settings->GetHologramInvalidColor();
			break;
		case EHologramMaterialState::HMS_OK:
		default:
			Color = Settings->GetHologramBuildColor();
			break;
		}
	}
	return FLinearColor(Color.X, Color.Y, Color.Z, 1.0f);
}

