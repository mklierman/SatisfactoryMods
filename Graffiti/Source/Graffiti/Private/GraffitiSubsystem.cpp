#include "GraffitiSubsystem.h"

#include "Components/DecalComponent.h"
#include "EngineUtils.h"
#include "FGIconDatabaseSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GraffitiConstants.h"
#include "GraffitiDecalActor.h"
#include "GraffitiIconUtils.h"
#include "GraffitiPlacementMath.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "Subsystem/SubsystemActorManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogGraffitiSubsystem, Log, All);

namespace
{
	constexpr float SpatialCellSize = 10000.0f;
	constexpr float ActivationDistance = 30000.0f;
	constexpr float DeactivationDistance = 35000.0f;
	constexpr int32 MaxActivationsPerTick = 64;
	constexpr int32 MaxPooledDecals = 512;
}

void FGraffitiRecordArray::PostReplicatedAdd(const TArrayView<int32>&, int32)
{
	if (Owner)
	{
		Owner->HandleReplicatedRecordsChanged();
	}
}

void FGraffitiRecordArray::PostReplicatedChange(const TArrayView<int32>&, int32)
{
	if (Owner)
	{
		Owner->HandleReplicatedRecordsChanged();
	}
}

void FGraffitiRecordArray::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32)
{
	if (Owner)
	{
		Owner->HandleReplicatedRecordsRemoving(RemovedIndices);
	}
}

AGraffitiSubsystem::AGraffitiSubsystem()
{
	ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnServer_Replicate;
	bAlwaysRelevant = true;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.25f;
	GraffitiRecords.Owner = this;
}

void AGraffitiSubsystem::Init()
{
	Super::Init();
	GraffitiRecords.Owner = this;
	bRuntimeIndexDirty = true;
}

AGraffitiSubsystem* AGraffitiSubsystem::Get(const UObject* WorldContext)
{
	if (!IsValid(WorldContext) || !IsValid(WorldContext->GetWorld()))
	{
		return nullptr;
	}

	if (USubsystemActorManager* Manager = WorldContext->GetWorld()->GetSubsystem<USubsystemActorManager>())
	{
		if (AGraffitiSubsystem* Registered = Manager->GetSubsystemActor<AGraffitiSubsystem>())
		{
			return Registered;
		}
	}

	for (TActorIterator<AGraffitiSubsystem> It(WorldContext->GetWorld()); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

uint64 AGraffitiSubsystem::AddGraffiti(
	const FTransform& Transform,
	const FPersistentGlobalIconId& IconId,
	const FLinearColor& Tint,
	float Size,
	bool bMirrorHorizontal)
{
	if (!HasAuthority())
	{
		return 0;
	}

	EnsureRuntimeIndex();
	const float ClampedSize = FMath::Clamp(Size, Graffiti::MinSize, Graffiti::MaxSize);
	const FVector Location = Transform.GetLocation();
	const FVector SurfaceNormal = Transform.GetUnitAxis(EAxis::X);
	int32 LayerOrder = 1;

	TArray<int32> NearbyRecords;
	FindRecordsNear(Location, ClampedSize + Graffiti::MaxSize, NearbyRecords);
	for (const int32 RecordIndex : NearbyRecords)
	{
		if (!GraffitiRecords.Items.IsValidIndex(RecordIndex))
		{
			continue;
		}

		const FGraffitiRecord& Existing = GraffitiRecords.Items[RecordIndex];
		if (GraffitiPlacementMath::CanDecalsOverlap(
			Location,
			SurfaceNormal,
			ClampedSize,
			Existing.Transform.GetLocation(),
			Existing.Transform.GetUnitAxis(EAxis::X),
			Existing.Size,
			Graffiti::LayerNormalAlignment,
			Graffiti::LayerPlaneTolerance,
			Graffiti::SquareRadiusScale) && Existing.LayerOrder < MAX_int32)
		{
			LayerOrder = FMath::Max(LayerOrder, Existing.LayerOrder + 1);
		}
	}

	const uint64 NewId = AddGraffitiInternal(Transform, IconId, Tint, ClampedSize, bMirrorHorizontal, LayerOrder);
	UE_LOG(LogGraffitiSubsystem, Display, TEXT("Added graffiti record %llu at %s"), NewId, *Location.ToCompactString());
	return NewId;
}

uint64 AGraffitiSubsystem::AddGraffitiInternal(
	const FTransform& Transform,
	const FPersistentGlobalIconId& IconId,
	const FLinearColor& Tint,
	float Size,
	bool bMirrorHorizontal,
	int32 LayerOrder)
{
	if (!HasAuthority())
	{
		return 0;
	}

	if (NextGraffitiId == 0)
	{
		NextGraffitiId = 1;
	}

	FGraffitiRecord& Record = GraffitiRecords.Items.AddDefaulted_GetRef();
	Record.Id = NextGraffitiId++;
	Record.Transform = Transform;
	Record.IconId = IconId;
	Record.Tint = Tint.GetClamped();
	Record.Size = FMath::Clamp(Size, Graffiti::MinSize, Graffiti::MaxSize);
	Record.bMirrorHorizontal = bMirrorHorizontal;
	Record.LayerOrder = FMath::Max(0, LayerOrder);
	GraffitiRecords.MarkItemDirty(Record);
	bRuntimeIndexDirty = true;
	ForceNetUpdate();
	return Record.Id;
}

bool AGraffitiSubsystem::RemoveClosestGraffiti(const FVector& Location)
{
	if (!HasAuthority())
	{
		return false;
	}

	EnsureRuntimeIndex();
	TArray<int32> NearbyRecords;
	FindRecordsNear(Location, Graffiti::MaxSize, NearbyRecords);

	int32 ClosestIndex = INDEX_NONE;
	float ClosestDistanceSquared = TNumericLimits<float>::Max();
	for (const int32 RecordIndex : NearbyRecords)
	{
		if (!GraffitiRecords.Items.IsValidIndex(RecordIndex))
		{
			continue;
		}

		const FGraffitiRecord& Record = GraffitiRecords.Items[RecordIndex];
		const float DistanceSquared = FVector::DistSquared(Record.Transform.GetLocation(), Location);
		const float EraseRadius = Record.Size * 0.6f;
		if (DistanceSquared <= FMath::Square(EraseRadius) && DistanceSquared < ClosestDistanceSquared)
		{
			ClosestIndex = RecordIndex;
			ClosestDistanceSquared = DistanceSquared;
		}
	}

	if (ClosestIndex == INDEX_NONE)
	{
		return false;
	}

	const uint64 RemovedId = GraffitiRecords.Items[ClosestIndex].Id;
	GraffitiRecords.Items.RemoveAt(ClosestIndex);
	GraffitiRecords.MarkArrayDirty();
	ReleaseDecal(RemovedId);
	bRuntimeIndexDirty = true;
	ForceNetUpdate();
	return true;
}

void AGraffitiSubsystem::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority() && !bLegacyMigrationAttempted)
	{
		MigrateLegacyActors();
	}

	if (GetNetMode() != NM_DedicatedServer)
	{
		RefreshVisibleDecals();
	}
}

void AGraffitiSubsystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ReleaseAllDecals();
	Super::EndPlay(EndPlayReason);
}

void AGraffitiSubsystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGraffitiSubsystem, GraffitiRecords);
}

void AGraffitiSubsystem::PostLoadGame_Implementation(int32, int32)
{
	GraffitiRecords.Owner = this;
	GraffitiRecords.Items.RemoveAll([](const FGraffitiRecord& Record)
	{
		return Record.Id == 0 || Record.IconId.IconID < 0 || !Record.Transform.IsValid() ||
			!FMath::IsFinite(Record.Size) || Record.Size < Graffiti::MinSize || Record.Size > Graffiti::MaxSize;
	});

	uint64 HighestId = 0;
	for (const FGraffitiRecord& Record : GraffitiRecords.Items)
	{
		HighestId = FMath::Max(HighestId, Record.Id);
	}
	NextGraffitiId = FMath::Max(NextGraffitiId, HighestId + 1);
	bRuntimeIndexDirty = true;
}

void AGraffitiSubsystem::GatherDependencies_Implementation(TArray<UObject*>&) {}

bool AGraffitiSubsystem::NeedTransform_Implementation()
{
	return false;
}

bool AGraffitiSubsystem::ShouldSave_Implementation() const
{
	return HasAuthority();
}

void AGraffitiSubsystem::HandleReplicatedRecordsChanged()
{
	bRuntimeIndexDirty = true;
	for (const TPair<uint64, TObjectPtr<UDecalComponent>>& Pair : ActiveDecals)
	{
		if (Pair.Value)
		{
			// A changed record may alter any visual field; the next visibility refresh reapplies it.
			Pair.Value->SetVisibility(false);
		}
	}
}

void AGraffitiSubsystem::HandleReplicatedRecordsRemoving(const TArrayView<int32>& RemovedIndices)
{
	for (const int32 RemovedIndex : RemovedIndices)
	{
		if (GraffitiRecords.Items.IsValidIndex(RemovedIndex))
		{
			ReleaseDecal(GraffitiRecords.Items[RemovedIndex].Id);
		}
	}
	bRuntimeIndexDirty = true;
}

void AGraffitiSubsystem::RebuildRuntimeIndex()
{
	IdToRecordIndex.Reset();
	SpatialCells.Reset();
	for (int32 Index = 0; Index < GraffitiRecords.Items.Num(); ++Index)
	{
		const FGraffitiRecord& Record = GraffitiRecords.Items[Index];
		IdToRecordIndex.Add(Record.Id, Index);
		SpatialCells.FindOrAdd(GetSpatialCell(Record.Transform.GetLocation())).Add(Record.Id);
	}
	bRuntimeIndexDirty = false;
}

void AGraffitiSubsystem::EnsureRuntimeIndex()
{
	if (bRuntimeIndexDirty)
	{
		RebuildRuntimeIndex();
	}
}

AGraffitiSubsystem::FSpatialCell AGraffitiSubsystem::GetSpatialCell(const FVector& Location) const
{
	return FSpatialCell(
		FMath::FloorToInt(Location.X / SpatialCellSize),
		FMath::FloorToInt(Location.Y / SpatialCellSize));
}

void AGraffitiSubsystem::FindRecordsNear(const FVector& Location, float Radius, TArray<int32>& OutRecordIndices) const
{
	OutRecordIndices.Reset();
	const FSpatialCell Center = GetSpatialCell(Location);
	const int32 CellRadius = FMath::CeilToInt(Radius / SpatialCellSize);
	for (int32 X = Center.X - CellRadius; X <= Center.X + CellRadius; ++X)
	{
		for (int32 Y = Center.Y - CellRadius; Y <= Center.Y + CellRadius; ++Y)
		{
			if (const TSet<uint64>* CellIds = SpatialCells.Find(FSpatialCell(X, Y)))
			{
				for (const uint64 Id : *CellIds)
				{
					if (const int32* RecordIndex = IdToRecordIndex.Find(Id))
					{
						OutRecordIndices.Add(*RecordIndex);
					}
				}
			}
		}
	}
}

void AGraffitiSubsystem::RefreshVisibleDecals()
{
	EnsureRuntimeIndex();
	APlayerController* Controller = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!Controller)
	{
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);

	TArray<uint64> DecalsToRelease;
	for (const TPair<uint64, TObjectPtr<UDecalComponent>>& Pair : ActiveDecals)
	{
		const int32* RecordIndex = IdToRecordIndex.Find(Pair.Key);
		if (!RecordIndex || !GraffitiRecords.Items.IsValidIndex(*RecordIndex) ||
			FVector::DistSquared(ViewLocation, GraffitiRecords.Items[*RecordIndex].Transform.GetLocation()) > FMath::Square(DeactivationDistance))
		{
			DecalsToRelease.Add(Pair.Key);
		}
		else if (Pair.Value && !Pair.Value->IsVisible())
		{
			// Icon data can arrive after the replicated records. Retry hidden decals until it is available.
			ConfigureDecal(Pair.Value, GraffitiRecords.Items[*RecordIndex]);
		}
	}
	for (const uint64 Id : DecalsToRelease)
	{
		ReleaseDecal(Id);
	}

	TArray<int32> NearbyRecords;
	FindRecordsNear(ViewLocation, ActivationDistance, NearbyRecords);
	int32 ActivationsRemaining = MaxActivationsPerTick;
	int32 ActivatedCount = 0;
	for (const int32 RecordIndex : NearbyRecords)
	{
		if (ActivationsRemaining <= 0 || !GraffitiRecords.Items.IsValidIndex(RecordIndex))
		{
			break;
		}

		const FGraffitiRecord& Record = GraffitiRecords.Items[RecordIndex];
		if (ActiveDecals.Contains(Record.Id) ||
			FVector::DistSquared(ViewLocation, Record.Transform.GetLocation()) > FMath::Square(ActivationDistance))
		{
			continue;
		}

		if (UDecalComponent* Decal = AcquireDecal())
		{
			ConfigureDecal(Decal, Record);
			ActiveDecals.Add(Record.Id, Decal);
			--ActivationsRemaining;
			++ActivatedCount;
		}
		else
		{
			break;
		}
	}

	if (ActivatedCount > 0)
	{
		UE_LOG(LogGraffitiSubsystem, Display, TEXT("Activated %d pooled decals (%d active, %d records)"),
			ActivatedCount, ActiveDecals.Num(), GraffitiRecords.Items.Num());
	}
}

void AGraffitiSubsystem::ConfigureDecal(UDecalComponent* Decal, const FGraffitiRecord& Record)
{
	if (!Decal)
	{
		return;
	}

	if (!DecalMaterial)
	{
		DecalMaterial = LoadObject<UMaterialInterface>(nullptr, Graffiti::DecalMaterialPath);
	}
	if (!DecalMaterial)
	{
		Decal->SetVisibility(false);
		return;
	}

	UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(Decal->GetDecalMaterial());
	if (!DynamicMaterial)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(DecalMaterial, Decal);
		Decal->SetDecalMaterial(DynamicMaterial);
	}

	UTexture* IconTexture = Graffiti::ResolveIconTexture(this, Record.IconId);
	if (!IconTexture)
	{
		Decal->SetVisibility(false);
		return;
	}

	DynamicMaterial->SetTextureParameterValue(Graffiti::IconTextureParameter, IconTexture);
	DynamicMaterial->SetVectorParameterValue(Graffiti::TintParameter, Record.Tint);
	DynamicMaterial->SetScalarParameterValue(Graffiti::MirrorParameter, Record.bMirrorHorizontal ? 1.0f : 0.0f);
	Decal->SetWorldTransform(Record.Transform);
	Decal->DecalSize = FVector(Graffiti::DecalDepth, Record.Size * 0.5f, Record.Size * 0.5f);
	Decal->FadeScreenSize = Graffiti::PreviewFadeScreenSize;
	Decal->SetSortOrder(Record.LayerOrder);
	Decal->SetVisibility(true);
	Decal->MarkRenderStateDirty();
}

UDecalComponent* AGraffitiSubsystem::AcquireDecal()
{
	while (!DecalPool.IsEmpty())
	{
		if (UDecalComponent* Decal = DecalPool.Pop(EAllowShrinking::No))
		{
			return Decal;
		}
	}

	if (!DecalMaterial)
	{
		DecalMaterial = LoadObject<UMaterialInterface>(nullptr, Graffiti::DecalMaterialPath);
	}
	if (!DecalMaterial)
	{
		UE_LOG(LogGraffitiSubsystem, Error, TEXT("Could not load graffiti decal material"));
		return nullptr;
	}

	UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(
		this,
		DecalMaterial,
		FVector(Graffiti::DecalDepth, 1.0f, 1.0f),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		0.0f);
	if (!Decal)
	{
		UE_LOG(LogGraffitiSubsystem, Error, TEXT("Could not create pooled graffiti decal component"));
		return nullptr;
	}

	Decal->SetVisibility(false);
	return Decal;
}

void AGraffitiSubsystem::ReleaseDecal(uint64 Id)
{
	TObjectPtr<UDecalComponent> Decal;
	if (!ActiveDecals.RemoveAndCopyValue(Id, Decal) || !Decal)
	{
		return;
	}

	Decal->SetVisibility(false);
	if (DecalPool.Num() < MaxPooledDecals)
	{
		DecalPool.Add(Decal);
	}
	else
	{
		Decal->DestroyComponent();
	}
}

void AGraffitiSubsystem::ReleaseAllDecals()
{
	for (const TPair<uint64, TObjectPtr<UDecalComponent>>& Pair : ActiveDecals)
	{
		if (Pair.Value)
		{
			Pair.Value->DestroyComponent();
		}
	}
	for (UDecalComponent* Decal : DecalPool)
	{
		if (Decal)
		{
			Decal->DestroyComponent();
		}
	}
	ActiveDecals.Reset();
	DecalPool.Reset();
}

void AGraffitiSubsystem::MigrateLegacyActors()
{
	bLegacyMigrationAttempted = true;
	int32 MigratedCount = 0;
	for (TActorIterator<AGraffitiDecalActor> It(GetWorld()); It; ++It)
	{
		AGraffitiDecalActor* LegacyActor = *It;
		if (!IsValid(LegacyActor) || LegacyActor->GetIconId().IconID < 0)
		{
			continue;
		}

		AddGraffitiInternal(
			LegacyActor->GetActorTransform(),
			LegacyActor->GetIconId(),
			LegacyActor->GetTint(),
			LegacyActor->GetGraffitiSize(),
			LegacyActor->IsMirroredHorizontally(),
			LegacyActor->GetLayerOrder());
		LegacyActor->Destroy();
		++MigratedCount;
	}

	if (MigratedCount > 0)
	{
		UE_LOG(LogGraffitiSubsystem, Log, TEXT("Migrated %d legacy graffiti actors into compact records"), MigratedCount);
	}
}

