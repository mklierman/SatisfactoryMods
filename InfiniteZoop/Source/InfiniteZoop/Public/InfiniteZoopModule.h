#pragma once

#include "Modules/ModuleManager.h"
#include "Hologram/FGHologram.h"
#include "Hologram/FGFactoryBuildingHologram.h"
#include "Equipment/FGBuildGunBuild.h"
#include "Hologram/FGFoundationHologram.h"
#include "InfiniteZoopSubsystem.h"

struct FZoopStruct
{
		int32 X = 0;
		int32 Y = 0;
		int32 Z = 0;
		//FIntVector LatestZoop = FIntVector(0, 0, 0);
		bool firstPassComplete = false;
		bool secondPassComplete = false;
		bool inScrollMode = false;
		EHologramMaterialState hgMaterialState1 = EHologramMaterialState::HMS_OK;
		EHologramMaterialState hgMaterialState2 = EHologramMaterialState::HMS_OK;
};

DECLARE_LOG_CATEGORY_EXTERN(InfiniteZoop_Log, Display, All);
class FInfiniteZoopModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	void ScrollHologram(AFGHologram* self, int32 delta);

	bool SetZoopAmount(AFGFactoryBuildingHologram* self, const FIntVector& Zoop);

	void OnZoopUpdated(UFGBuildGunStateBuild* self, float currentZoop, float maxZoop, const FVector& zoopLocation);

	bool OnRep_DesiredZoop(AFGFactoryBuildingHologram* self);

	int32 GetBaseCostMultiplier(const AFGFactoryBuildingHologram* self);

	void CreateDefaultFoundationZoop(AFGFoundationHologram* self, const FHitResult& hitResult);

	void ConstructZoop(AFGFoundationHologram* self, TArray<AActor*>& out_children);

	FZoopStruct* GetStruct(AFGFoundationHologram* self);

	bool IsZoopMode(AFGFoundationHologram* self);

	bool BGSecondaryFire(UFGBuildGunStateBuild* self);

	bool ValidatePlacementAndCost(AFGHologram* self, class UFGInventoryComponent* inventory);

	bool SetMaterialState(AFGHologram* self, EHologramMaterialState state);

	TMap< AFGFactoryBuildingHologram*, int32> LastMultiplier;
	TMap<AFGHologram*, FIntVector> HologramsToZoop;
	TMap<AFGFoundationHologram*, FZoopStruct*> FoundationsBeingZooped;

	FVector CalcPivotAxis(const EAxis::Type DesiredAxis, const FVector& ViewVector, const FQuat& ActorQuat);

	void SetSubsystemZoopAmounts(int x, int y, int z, bool isFoundation, UWorld* world, AFGHologram* hologram);

	AInfiniteZoopSubsystem* zoopSubsystem;
};