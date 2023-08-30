#pragma once

#include "Hologram/FGBuildableHologram.h"
#include "Equipment/FGBuildGunBuild.h"
#include "Patching/NativeHookManager.h"
#include "Hologram/FGHologram.h"
#include "../../../../SML/Source/SML/Public/Patching/NativeHookManager.h"
#include "../../../../../Source/FactoryGame/Public/Hologram/FGBuildableHologram.h"
#include "Modules/ModuleManager.h"

class FInfiniteNudgeModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	FVector NudgeTowardsWorldDirection(AFGHologram* self, const FVector& Direction);

	float savedNudgeDistance;

	float LeftCtrlNudgeAmount = 20.f;
	float LeftAltNudgeAmount = 50.f;
};