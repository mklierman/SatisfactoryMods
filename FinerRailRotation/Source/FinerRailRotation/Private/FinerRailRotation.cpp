// Copyright Epic Games, Inc. All Rights Reserved.

#include "FinerRailRotation.h"
#include "Hologram/FGRailroadTrackHologram.h"
#include "Patching/NativeHookManager.h"
#include "FGPlayerController.h"

#define LOCTEXT_NAMESPACE "FFinerRailRotationModule"

void FFinerRailRotationModule::StartupModule()
{
#if !WITH_EDITOR
	AFGRailroadTrackHologram* rrh = GetMutableDefault<AFGRailroadTrackHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGRailroadTrackHologram::GetRotationStep, rrh, [this](auto& scope, const AFGRailroadTrackHologram* self)
		{
			auto inst = self->GetConstructionInstigator();
			auto cont = Cast<AFGPlayerController>(inst->GetController());
			if (cont && (cont->IsInputKeyDown(EKeys::LeftControl) || cont->IsInputKeyDown(EKeys::RightControl)))
			{
				scope.Override(1);
			}
		});
#endif
}

void FFinerRailRotationModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFinerRailRotationModule, FinerRailRotation)