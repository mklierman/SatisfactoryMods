#pragma once

#include "Modules/ModuleManager.h"

class AFGHoverPack;
class AFGBuildableRailroadTrack;
class UFGPowerConnectionComponent;

class FHoverPackPoleRangeModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual bool IsGameModule() const override { return true; }

private:
	float GetScaledRange(AFGHoverPack* hoverPack, UFGPowerConnectionComponent* powerConnection, AFGBuildableRailroadTrack* railroadTrack, float nativeSearchRadius) const;

	void UpdateSearchRadius(AFGHoverPack* hoverPack);
	bool TryConnectToBestPowerConnection(AFGHoverPack* hoverPack);

	struct FHoverPackRangeState
	{
		float nativeSearchRadius = 0.0f;
		float lastAppliedSearchRadius = 0.0f;
	};

	TMap<TWeakObjectPtr<AFGHoverPack>, FHoverPackRangeState> mRangeStates;
};
