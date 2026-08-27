#pragma once

#include "CoreMinimal.h"
#include "FGSaveInterface.h"
#include "HypertubeDestinationTypes.h"
#include "Subsystem/ModSubsystem.h"
#include "HypertubeDestinationSubsystem.generated.h"

class AFGCharacterPlayer;
class AFGBuildablePipeHyperJunction;
class AFGPipeHyperStart;
class UFGPipeConnectionComponentBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHypertubeDestinationsChanged);

UCLASS(Blueprintable)
class HYPERTUBEDESTINATIONS_API AHypertubeDestinationSubsystem : public AModSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()

public:
	AHypertubeDestinationSubsystem();

	static AHypertubeDestinationSubsystem* Get(const UObject* WorldContext);

	UFUNCTION(BlueprintPure, Category = "Hypertube Destinations", meta = (WorldContext = "WorldContext"))
	static AHypertubeDestinationSubsystem* GetHypertubeDestinationSubsystem(const UObject* WorldContext);

	UFUNCTION(BlueprintCallable, Category = "Hypertube Destinations")
	FGuid RegisterEntrance(AFGPipeHyperStart* Entrance);

	UFUNCTION(BlueprintCallable, Category = "Hypertube Destinations")
	void MarkEntranceUnavailable(AFGPipeHyperStart* Entrance);

	/** Permanently removes an entrance record. Called by the native dismantle hook, not the mixin's EndPlay event. */
	UFUNCTION(BlueprintCallable, Category = "Hypertube Destinations")
	bool RemoveEntrance(AFGPipeHyperStart* Entrance);

	UFUNCTION(BlueprintCallable, Category = "Hypertube Destinations")
	bool SetDestinationName(AFGPipeHyperStart* Entrance, const FString& NewName);

	UFUNCTION(BlueprintPure, Category = "Hypertube Destinations")
	TArray<FHypertubeDestinationRecord> GetAvailableDestinations() const;

	UFUNCTION(BlueprintPure, Category = "Hypertube Destinations")
	bool GetDestinationForEntrance(AFGPipeHyperStart* Entrance, FHypertubeDestinationRecord& OutDestination) const;

	bool SelectDestination(AFGCharacterPlayer* Player, AFGPipeHyperStart* Source, AFGPipeHyperStart* Destination);
	bool PrepareJunctionRoute(AFGCharacterPlayer* Player, AFGBuildablePipeHyperJunction* Junction, UFGPipeConnectionComponentBase* IncomingConnection, UFGPipeConnectionComponentBase*& OutgoingConnection);
	bool TryRouteJunctionExit(AFGCharacterPlayer* Player, AFGBuildablePipeHyperJunction* Junction, UFGPipeConnectionComponentBase* VanillaExit, UFGPipeConnectionComponentBase*& OutExit);
	const FHypertubeRoutePlan* GetActiveRoute(AFGCharacterPlayer* Player) const;
	void ClearActiveRoute(AFGCharacterPlayer* Player);

	UPROPERTY(BlueprintAssignable, Category = "Hypertube Destinations")
	FHypertubeDestinationsChanged OnDestinationsChanged;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreSaveGame_Implementation(int32 SaveVersion, int32 GameVersion) override;
	virtual void PostSaveGame_Implementation(int32 SaveVersion, int32 GameVersion) override;
	virtual void PreLoadGame_Implementation(int32 SaveVersion, int32 GameVersion) override;
	virtual void PostLoadGame_Implementation(int32 SaveVersion, int32 GameVersion) override;
	virtual void GatherDependencies_Implementation(TArray<UObject*>& OutDependentObjects) override;
	virtual bool ShouldSave_Implementation() const override;
	virtual bool NeedTransform_Implementation() override;

protected:
	virtual void Init() override;

private:
	UFUNCTION()
	void OnRep_Destinations();

	FHypertubeDestinationRecord* FindMutableDestination(AFGPipeHyperStart* Entrance);
	const FHypertubeDestinationRecord* FindDestination(AFGPipeHyperStart* Entrance) const;

	UPROPERTY(SaveGame, ReplicatedUsing = OnRep_Destinations)
	TArray<FHypertubeDestinationRecord> Destinations;

	TMap<TWeakObjectPtr<AFGCharacterPlayer>, FHypertubeRoutePlan> ActiveRoutes;
	int32 TopologyRevision = 0;
};
