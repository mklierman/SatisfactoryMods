#include "GraffitiDecalActor.h"

#include "Components/DecalComponent.h"
#include "FGIconDatabaseSubsystem.h"
#include "GraffitiConstants.h"
#include "GraffitiIconUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogGraffitiDecal, Log, All);

AGraffitiDecalActor::AGraffitiDecalActor()
{
	bReplicates = true;
	bAlwaysRelevant = false;
	SetNetCullDistanceSquared(FMath::Square(30000.0f));
	SetReplicateMovement(false);

	DecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("GraffitiDecal"));
	SetRootComponent(DecalComponent);
	DecalComponent->DecalSize = FVector(Graffiti::DecalDepth, Size * 0.5f, Size * 0.5f);
	DecalComponent->FadeScreenSize = Graffiti::PreviewFadeScreenSize;
}

void AGraffitiDecalActor::BeginPlay()
{
	Super::BeginPlay();
	RefreshDecal();

	if (AFGIconDatabaseSubsystem* Database = AFGIconDatabaseSubsystem::Get(this); Database && !Database->IsInitialized())
	{
		Database->mOnDatabaseAvailable.AddUniqueDynamic(this, &AGraffitiDecalActor::OnRep_GraffitiData);
	}
}

void AGraffitiDecalActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGraffitiDecalActor, IconId);
	DOREPLIFETIME(AGraffitiDecalActor, Tint);
	DOREPLIFETIME(AGraffitiDecalActor, Size);
	DOREPLIFETIME(AGraffitiDecalActor, bMirrorHorizontal);
	DOREPLIFETIME(AGraffitiDecalActor, LayerOrder);
}

void AGraffitiDecalActor::InitializeGraffiti(
	const FPersistentGlobalIconId& InIconId,
	const FLinearColor& InTint,
	float InSize,
	bool bInMirrorHorizontal,
	int32 InLayerOrder)
{
	check(HasAuthority());
	IconId = InIconId;
	Tint = InTint;
	Size = FMath::Clamp(InSize, Graffiti::MinSize, Graffiti::MaxSize);
	bMirrorHorizontal = bInMirrorHorizontal;
	LayerOrder = FMath::Max(0, InLayerOrder);
	RefreshDecal();
	ForceNetUpdate();
}

void AGraffitiDecalActor::OnRep_GraffitiData()
{
	RefreshDecal();
}

void AGraffitiDecalActor::RefreshDecal()
{
	if (!DecalMaterial)
	{
		DecalMaterial = LoadObject<UMaterialInterface>(nullptr, Graffiti::DecalMaterialPath);
	}

	if (!DecalComponent || !DecalMaterial)
	{
		return;
	}

	AFGIconDatabaseSubsystem* Database = AFGIconDatabaseSubsystem::Get(this);
	if (!Database || !Database->IsInitialized())
	{
		return;
	}

	UTexture* IconTexture = Graffiti::ResolveIconTexture(this, IconId);
	if (!IconTexture)
	{
		UE_LOG(LogGraffitiDecal, Warning, TEXT("Could not resolve graffiti icon texture"));
		DecalComponent->SetVisibility(false);
		return;
	}

	if (!DynamicMaterial)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(DecalMaterial, this);
		DecalComponent->SetDecalMaterial(DynamicMaterial);
	}

	DynamicMaterial->SetTextureParameterValue(Graffiti::IconTextureParameter, IconTexture);
	DynamicMaterial->SetVectorParameterValue(Graffiti::TintParameter, Tint);
	DynamicMaterial->SetScalarParameterValue(Graffiti::MirrorParameter, bMirrorHorizontal ? 1.0f : 0.0f);
	DecalComponent->DecalSize = FVector(Graffiti::DecalDepth, Size * 0.5f, Size * 0.5f);
	DecalComponent->SetSortOrder(LayerOrder);
	DecalComponent->SetVisibility(true);
	DecalComponent->MarkRenderStateDirty();
}

void AGraffitiDecalActor::PreSaveGame_Implementation(int32 SaveVersion, int32 GameVersion) {}
void AGraffitiDecalActor::PostSaveGame_Implementation(int32 SaveVersion, int32 GameVersion) {}
void AGraffitiDecalActor::PreLoadGame_Implementation(int32 SaveVersion, int32 GameVersion) {}

void AGraffitiDecalActor::PostLoadGame_Implementation(int32 SaveVersion, int32 GameVersion)
{
	RefreshDecal();
}

void AGraffitiDecalActor::GatherDependencies_Implementation(TArray<UObject*>& OutDependentObjects) {}
bool AGraffitiDecalActor::NeedTransform_Implementation() { return true; }
bool AGraffitiDecalActor::ShouldSave_Implementation() const { return true; }
