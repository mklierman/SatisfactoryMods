#pragma once

#include "CoreMinimal.h"
#include "FGIconLibrary.h"
#include "FGSaveInterface.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Subsystem/ModSubsystem.h"
#include "GraffitiSubsystem.generated.h"

class AGraffitiSubsystem;
class AFGBuildableGraffitiContainer;
class UDecalComponent;

USTRUCT()
struct GRAFFITI_API FGraffitiRecord : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	uint64 Id = 0;

	UPROPERTY(SaveGame)
	FTransform Transform = FTransform::Identity;

	UPROPERTY(SaveGame)
	FPersistentGlobalIconId IconId;

	UPROPERTY(SaveGame)
	float Size = 200.0f;

	UPROPERTY(SaveGame)
	bool bMirrorHorizontal = false;

	UPROPERTY(SaveGame)
	int32 LayerOrder = 0;

	UPROPERTY(SaveGame)
	TObjectPtr<AFGBuildableGraffitiContainer> SourceContainer;

	UPROPERTY(SaveGame)
	FGuid SourceEntryId;
};

USTRUCT()
struct GRAFFITI_API FGraffitiRecordArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TArray<FGraffitiRecord> Items;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParameters)
	{
		return FastArrayDeltaSerialize<FGraffitiRecord, FGraffitiRecordArray>(Items, DeltaParameters, *this);
	}

	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);
	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);

	AGraffitiSubsystem* Owner = nullptr;
};

template<>
struct TStructOpsTypeTraits<FGraffitiRecordArray> : public TStructOpsTypeTraitsBase2<FGraffitiRecordArray>
{
	enum { WithNetDeltaSerializer = true };
};

UCLASS(Blueprintable)
class GRAFFITI_API AGraffitiSubsystem : public AModSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()

public:
	AGraffitiSubsystem();

	static AGraffitiSubsystem* Get(const UObject* WorldContext);

	uint64 AddGraffiti(
		const FTransform& Transform,
		const FPersistentGlobalIconId& IconId,
		float Size,
		bool bMirrorHorizontal);

	bool RemoveClosestGraffiti(const FVector& Location);

	uint64 RegisterContainerEntry(
		AFGBuildableGraffitiContainer* Container,
		const FGuid& EntryId,
		const FTransform& Transform,
		const FPersistentGlobalIconId& IconId,
		float Size,
		bool bMirrorHorizontal,
		int32& InOutLayerOrder);

	bool UnregisterContainerEntry(AFGBuildableGraffitiContainer* Container, const FGuid& EntryId);

	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostLoadGame_Implementation(int32 SaveVersion, int32 GameVersion) override;
	virtual void GatherDependencies_Implementation(TArray<UObject*>& OutDependentObjects) override;
	virtual bool NeedTransform_Implementation() override;
	virtual bool ShouldSave_Implementation() const override;

	void HandleReplicatedRecordsChanged();
	void HandleReplicatedRecordsRemoving(const TArrayView<int32>& RemovedIndices);

protected:
	virtual void Init() override;

private:
	using FSpatialCell = FIntPoint;

	uint64 AddGraffitiInternal(
		const FTransform& Transform,
		const FPersistentGlobalIconId& IconId,
		float Size,
		bool bMirrorHorizontal,
		int32 LayerOrder,
		AFGBuildableGraffitiContainer* SourceContainer = nullptr,
		const FGuid& SourceEntryId = FGuid());

	int32 CalculateLayerOrder(const FTransform& Transform, float Size);
	int32 FindContainerRecordIndex(const AFGBuildableGraffitiContainer* Container, const FGuid& EntryId) const;

	void RebuildRuntimeIndex();
	void EnsureRuntimeIndex();
	FSpatialCell GetSpatialCell(const FVector& Location) const;
	void FindRecordsNear(const FVector& Location, float Radius, TArray<int32>& OutRecordIndices) const;
	void RefreshVisibleDecals();
	void ConfigureDecal(UDecalComponent* Decal, const FGraffitiRecord& Record);
	UDecalComponent* AcquireDecal();
	void ReleaseDecal(uint64 Id);
	void ReleaseAllDecals();
	void MigrateLegacyActors();

	UPROPERTY(SaveGame, Replicated)
	FGraffitiRecordArray GraffitiRecords;

	UPROPERTY(SaveGame)
	uint64 NextGraffitiId = 1;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> DecalMaterial;

	UPROPERTY(Transient)
	TMap<uint64, TObjectPtr<UDecalComponent>> ActiveDecals;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UDecalComponent>> DecalPool;

	TMap<uint64, int32> IdToRecordIndex;
	TMap<FSpatialCell, TSet<uint64>> SpatialCells;
	bool bRuntimeIndexDirty = true;
	bool bLegacyMigrationAttempted = false;
	bool bInitialActivationComplete = false;
};

