
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FGSaveInterface.h"
#include "FGFactoryClipboard.h"
#include "CTBP_Component.generated.h"

USTRUCT(BlueprintType, Category = "CopyToBP")
struct FClipboardData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadWrite)
	TSubclassOf<UObject> ObjectClass;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	TSubclassOf<UFGFactoryClipboardSettings> SettingsClass;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	TSubclassOf<class UFGRecipe> Recipe;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	float TargetPotential = 1.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	float TargetProductionBoost = 1.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	float ReachablePotential = 1.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	float ReachableProductionBoost = 1.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	TSubclassOf<class UFGPowerShardDescriptor> OverclockingShardDescriptor;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	TSubclassOf<class UFGPowerShardDescriptor> ProductionBoostShardDescriptor;

	UPROPERTY(SaveGame)
	TArray<uint8> SerializedSettings;
};

UCLASS(Blueprintable)
class COPYTOBP_API UCTBP_Component : public UActorComponent, public IFGSaveInterface
{
	GENERATED_BODY()
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreSaveGame_Implementation(int32 SaveVersion, int32 GameVersion) override;
	virtual void PostSaveGame_Implementation(int32 SaveVersion, int32 GameVersion) override {}
	virtual void PreLoadGame_Implementation(int32 SaveVersion, int32 GameVersion) override {}
	virtual void PostLoadGame_Implementation(int32 SaveVersion, int32 GameVersion) override;
	virtual void GatherDependencies_Implementation(TArray<UObject*>& OutDependentObjects) override {}
	virtual bool NeedTransform_Implementation() override { return false; }
	virtual bool ShouldSave_Implementation() const override { return true; }

public:	
	UCTBP_Component();

	static UCTBP_Component* FindOrAdd(class AFGPlayerState* PlayerState);
	static TArray<FClipboardData> MakeClipboardDataFromPlayerState(class AFGPlayerState* State);
	static void ApplyClipboardDataToPlayerState(class AFGPlayerState* State, const TArray<FClipboardData>& Data);

	void CaptureClipboardFromOwner();
	void ApplyClipboardToOwner();

protected:
	virtual void BeginPlay() override;

public:	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerSetShouldPasteToBP(bool bNewValue);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerSetPlayerClipboardData(const TArray<FClipboardData>& data);

	UFUNCTION()
	void OnRep_PlayerClipboardData();

	UPROPERTY(BlueprintReadWrite, Replicated, SaveGame)
	bool ShouldPasteToBP = true;

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_PlayerClipboardData, SaveGame)
	TArray<FClipboardData> PlayerClipboardData;
};
