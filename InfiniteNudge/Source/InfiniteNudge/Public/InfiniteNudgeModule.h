#pragma once

#include "Modules/ModuleManager.h"
#include "Hologram/FGHologram.h"

DECLARE_LOG_CATEGORY_EXTERN(InfiniteNudge, Verbose, All);

class FInfiniteNudgeModule : public FDefaultGameModuleImpl {

public:
	// Map of vanilla hologram classes to the nudgeable ones that override CanNudgeHologram. Updated by the GameInstanceModule on Initialize.
	static inline TMap<TSubclassOf<AFGHologram>, TSubclassOf<AFGHologram>> mHologramOverrides = {};
	static inline FVector mPivot = {};
	
	virtual void StartupModule() override;
	virtual bool IsGameModule() const override { return true; }

	static void RotateHologram(AFGHologram* self, int32 step);
	static void ScaleHologram(AFGHologram* self, int32 step);
};