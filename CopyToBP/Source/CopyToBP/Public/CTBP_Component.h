
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FGSaveInterface.h"
#include "CTBP_Component.generated.h"

USTRUCT(BlueprintType, Category = "CopyToBP")
struct FClipboardData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UObject> ObjectClass;

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<class UFGRecipe> Recipe;

	UPROPERTY(BlueprintReadWrite)
	float TargetPotential = 1.0f;

	UPROPERTY(BlueprintReadWrite)
	float TargetProductionBoost = 1.0f;

	UPROPERTY(BlueprintReadWrite)
	float ReachablePotential = 1.0f;

	UPROPERTY(BlueprintReadWrite)
	float ReachableProductionBoost = 1.0f;

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<class UFGPowerShardDescriptor> OverclockingShardDescriptor;

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<class UFGPowerShardDescriptor> ProductionBoostShardDescriptor;
};

UCLASS(Blueprintable)
class COPYTOBP_API UCTBP_Component : public UActorComponent, public IFGSaveInterface
{
	GENERATED_BODY()
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ShouldSave_Implementation() const override { return true; }

public:	
	UCTBP_Component();

	static UCTBP_Component* FindOrAdd(class AFGPlayerState* PlayerState);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerSetShouldPasteToBP(bool bNewValue);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerSetPlayerClipboardData(const TArray<FClipboardData>& data);

	UPROPERTY(BlueprintReadWrite, Replicated, SaveGame)
	bool ShouldPasteToBP = true;

	UPROPERTY(BlueprintReadWrite, Replicated)
	TArray<FClipboardData> PlayerClipboardData;
};
