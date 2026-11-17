#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildable.h"
#include "FGIconLibrary.h"
#include "FGBuildableGraffitiContainer.generated.h"

USTRUCT()
struct GRAFFITI_API FGraffitiContainerEntry
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FGuid EntryId;

	UPROPERTY(SaveGame)
	FTransform RelativeTransform = FTransform::Identity;

	UPROPERTY(SaveGame)
	FPersistentGlobalIconId IconId;

	UPROPERTY(SaveGame)
	float Size = 200.0f;

	UPROPERTY(SaveGame)
	bool bMirrorHorizontal = false;

	UPROPERTY(SaveGame)
	int32 LayerOrder = 0;
};

/**
 * Invisible buildable carrying a collection of graffiti through vanilla blueprint serialization.
 * Rendering, spatial lookup, and standalone replication remain owned by AGraffitiSubsystem.
 */
UCLASS()
class GRAFFITI_API AFGBuildableGraffitiContainer : public AFGBuildable
{
	GENERATED_BODY()

public:
	AFGBuildableGraffitiContainer(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	FGuid AddGraffiti(
		const FTransform& WorldTransform,
		const FPersistentGlobalIconId& IconId,
		float Size,
		bool bMirrorHorizontal);

	bool RemoveGraffitiEntry(const FGuid& EntryId);
	bool ContainsEntry(const FGuid& EntryId) const;
	const TArray<FGraffitiContainerEntry>& GetEntries() const { return Entries; }

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION()
	void OnRep_Entries();

	bool RegisterEntries();
	void UnregisterEntries();

	UPROPERTY(SaveGame, ReplicatedUsing = OnRep_Entries)
	TArray<FGraffitiContainerEntry> Entries;

	bool bEntriesRegistered = false;
};

