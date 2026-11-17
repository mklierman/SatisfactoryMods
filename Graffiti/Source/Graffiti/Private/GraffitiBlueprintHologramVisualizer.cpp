#include "GraffitiBlueprintHologramVisualizer.h"

#include "Components/DecalComponent.h"
#include "FGBuildableGraffitiContainer.h"
#include "GraffitiConstants.h"
#include "GraffitiIconUtils.h"
#include "Hologram/FGBlueprintHologram.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

UGraffitiBlueprintHologramVisualizer::UGraffitiBlueprintHologramVisualizer()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UGraffitiBlueprintHologramVisualizer::Initialize(
	AFGBlueprintHologram* BlueprintHologram,
	const AFGBuildableGraffitiContainer* Container,
	USceneComponent* BuildableRoot)
{
	if (!IsValid(BlueprintHologram) || !IsValid(Container) || !IsValid(BuildableRoot))
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
		UTexture* IconTexture = Graffiti::ResolveIconTexture(BlueprintHologram, Entry.IconId);
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
	}
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
	Super::EndPlay(EndPlayReason);
}

