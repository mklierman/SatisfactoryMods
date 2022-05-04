#pragma once

#include "Equipment/FGHoverPack.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(HoverPackPoleRange_Log, Display, All);
class FHoverPackPoleRangeModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	bool mAllowAdd;
	bool mAllowRemove = true;
	bool mConnectionAdded = true;

	int32 mMk1Range;
	int32 mMk2Range;
	int32 mMk3Range;
	int32 mRailRange;
	int32 mElseRange;

	TMap<AFGHoverPack*, bool> mAddedConnections;

	UFUNCTION()
		void Mk1Updated();

	void SetConfigValues();
};