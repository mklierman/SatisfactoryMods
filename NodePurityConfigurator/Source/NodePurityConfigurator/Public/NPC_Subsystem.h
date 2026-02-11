

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "Resources/FGResourceNode.h"
#include <FGSaveInterface.h>
#include "NPC_Subsystem.generated.h"

/**
 * 
 */
UCLASS()
class NODEPURITYCONFIGURATOR_API ANPC_Subsystem : public AModSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()

	void BeginPlay() override;
	bool ShouldSave_Implementation() const override;

	void FillPurityMaps();
public:
	UPROPERTY()
	TMap<int, int> IronPurities;
	
	UPROPERTY()
	TMap<int, int> CopperPurities;
	
	UPROPERTY()
	TMap<int, int> CoalPurities;
	
	UPROPERTY()
	TMap<int, int> LimestonePurities;

	UPROPERTY()
	TMap<int, int> CateriumPurities;
	
	UPROPERTY()
	TMap<int, int> BauxitePurities;
	
	UPROPERTY()
	TMap<int, int> OilPurities;
	
	UPROPERTY()
	TMap<int, int> OilWellPurities;
	
	UPROPERTY()
	TMap<int, int> SAMPurities;
	
	UPROPERTY()
	TMap<int, int> UraniumPurities;
	
	UPROPERTY()
	TMap<int, int> QuartzPurities;
	
	UPROPERTY()
	TMap<int, int> SulfurPurities;
	
	UPROPERTY()
	TMap<int, int> NitrogenWellPurities;
	
	UPROPERTY()
	TMap<int, int> WaterWellPurities;
	
	UPROPERTY(SaveGame)
	TMap<FVector, int> SavedPurities;
};
