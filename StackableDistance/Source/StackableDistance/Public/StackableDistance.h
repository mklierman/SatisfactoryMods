#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "SDRangeVisualizer.h"
#include "Hologram/FGBuildableHologram.h"
#include "Equipment/FGBuildGun.h"
#include "FGRecipe.h"
#include "GameFramework/Pawn.h"

DECLARE_LOG_CATEGORY_EXTERN(StackableDistance_Log, Display, All);

class FStackableDistanceModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool IsGameModule() const override { return true; }

	void OnBuildableConstructed(const AFGBuildableHologram* self, AFGBuildable* inBuildable);
	void HandleHologramPlacement(AFGHologram* self, const FHitResult& hitResult);
	void ResetAnchor();

private:
	float GetSplineMaxLengthForClass(UClass* buildableClass);
	void UpdateRangeVisualizer(AFGHologram* hologram, float baseDistance, float minCurveScale);
	void DestroyRangeVisualizer();
	bool IsHologramForAnchor(AFGHologram* hologram);
	bool IsRecipeForBuildClass(TSubclassOf<UFGRecipe> recipe, UClass* buildClass);
	bool IsRecipeForAnchor(TSubclassOf<UFGRecipe> recipe);
	static bool IsLocalPawn(APawn* pawn);
	static bool IsLocalHologram(const AFGHologram* hologram);
	static bool IsLocalBuildState(UFGBuildGunState* state);
	static bool IsPoleBuildClass(UClass* buildClass);
	void SetAnchorFromPole(FVector location, FRotator rotation, UClass* buildClass);

	bool HasAnchor = false;
	FVector LastPoleLocation = FVector::ZeroVector;
	FRotator LastPoleRotation = FRotator::ZeroRotator;
	float CurrentTargetMaxLength = 5600.0f;
	TWeakObjectPtr<UClass> LastPoleClass = nullptr;
	TWeakObjectPtr<USDRangeDrawComponent> RangeDraw = nullptr;
};
