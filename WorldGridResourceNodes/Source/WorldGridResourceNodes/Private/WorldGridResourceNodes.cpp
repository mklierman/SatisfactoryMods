// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorldGridResourceNodes.h"
#include "Patching/NativeHookManager.h"
#include "Resources/FGResourceNodeBase.h"
#include "Hologram/FGWaterPumpHologram.h"
#include <Logging/StructuredLog.h>

#define LOCTEXT_NAMESPACE "FWorldGridResourceNodesModule"
DEFINE_LOG_CATEGORY(WGRN_Log);

void FWorldGridResourceNodesModule::StartupModule()
{
#if !WITH_EDITOR
	AFGResourceNodeBase* rnb = GetMutableDefault<AFGResourceNodeBase>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGResourceNodeBase::GetPlacementLocation, rnb, [this](auto& scope, const AFGResourceNodeBase* node, const FVector& hitLocation)
		{
			FVector result = scope(node, hitLocation);
			result.X = FMath::RoundToFloat(result.X / 100.0f) * 100.0f;
			result.Y = FMath::RoundToFloat(result.Y / 100.0f) * 100.0f;
			scope.Override(result);
		});

	AFGWaterPumpHologram* wph = GetMutableDefault<AFGWaterPumpHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGWaterPumpHologram::TrySnapToExtractableResource, wph, [this](auto& scope, AFGWaterPumpHologram* self, const FHitResult& hitResult, FVector& newHitLocation)
		{
			scope(self, hitResult, newHitLocation);
			newHitLocation.X = FMath::RoundToFloat(newHitLocation.X / 100.0f) * 100.0f;
			newHitLocation.Y = FMath::RoundToFloat(newHitLocation.Y / 100.0f) * 100.0f;
			scope.Cancel();
		});
#endif
}

void FWorldGridResourceNodesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FWorldGridResourceNodesModule, WorldGridResourceNodes)