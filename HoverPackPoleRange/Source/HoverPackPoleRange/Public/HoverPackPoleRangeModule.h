#pragma once

#include "Equipment/FGHoverPack.h"
#include "Modules/ModuleManager.h"
#include "HPPR_Subsystem.h"

DECLARE_LOG_CATEGORY_EXTERN(HoverPackPoleRange_Log, Display, All);
class FHoverPackPoleRangeModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	UPROPERTY()
	int32 mMk1Range;
	UPROPERTY()
	int32 mMk2Range;
	UPROPERTY()
	int32 mMk3Range;
	UPROPERTY()
	int32 mRailRange;
	UPROPERTY()
	int32 mElseRange;

	UPROPERTY()
	TMap<AFGHoverPack*, bool> mAddedConnections;
	UPROPERTY()
	TMap<AFGHoverPack*, bool> mAllowRemove;

	UPROPERTY()
	AHPPR_Subsystem* mHPSubsystem;


	void Loggit(FString myString);

	UPROPERTY()
		bool debugLogging = false;
};