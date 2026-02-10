

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "Resources/FGResourceNode.h"
#include "NPC_Subsystem.generated.h"

/**
 * 
 */
UCLASS()
class NODEPURITYCONFIGURATOR_API ANPC_Subsystem : public AModSubsystem
{
	GENERATED_BODY()
	void BeginPlay() override;
public:
	UPROPERTY(SaveGame)
	TMap<int, int> IronPurities;
	
	UPROPERTY(SaveGame)
	TMap<int, int> CopperPurities;
	
	UPROPERTY(SaveGame)
	TMap<int, int> CoalPurities;
	
	UPROPERTY(SaveGame)
	TMap<int, int> LimestonePurities;
	
	UPROPERTY(SaveGame)
	TMap<int, int> BauxitePurities;
	
	UPROPERTY(SaveGame)
	TMap<int, int> OilPurities;
	
	UPROPERTY(SaveGame)
	TMap<int, int> OilWellPurities;
	
	UPROPERTY(SaveGame)
	TMap<int, int> SAMPurities;
	
	UPROPERTY(SaveGame)
	TMap<int, int> UraniumPurities;
	
	UPROPERTY(SaveGame)
	TMap<int, int> QuartzPurities;
	
	UPROPERTY(SaveGame)
	TMap<int, int> SulfurPurities;
	
	UPROPERTY(SaveGame)
	TMap<int, int> NitrogenWellPurities;
	
	UPROPERTY(SaveGame)
	TMap<int, int> WaterWellPurities;
	
	UPROPERTY(SaveGame)
	TMap<FVector, int> SavedPurities;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UFGResourceDescriptor> ironDescClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UFGResourceDescriptor> copperDescClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UFGResourceDescriptor> LimestoneDescClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UFGResourceDescriptor> CoalDescClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UFGResourceDescriptor> BauxiteDescClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UFGResourceDescriptor> QuartzDescClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UFGResourceDescriptor> UraniumDescClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UFGResourceDescriptor> SulfurDescClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UFGResourceDescriptor> OilDescClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UFGResourceDescriptor> WaterDescClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UFGResourceDescriptor> NitrogenDescClass;
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UFGResourceDescriptor> SAMDescClass;
};
