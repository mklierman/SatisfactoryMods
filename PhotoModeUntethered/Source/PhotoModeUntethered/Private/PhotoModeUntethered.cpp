// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhotoModeUntethered.h"
#include "FGPhotoModeComponent.h"
#include "Patching/NativeHookManager.h"

#define LOCTEXT_NAMESPACE "FPhotoModeUntetheredModule"

void FPhotoModeUntetheredModule::StartupModule()
{
#if !WITH_EDITOR
	SUBSCRIBE_METHOD(UFGPhotoModeComponent::ToggleDecoupledCamera, [this](auto& scope, UFGPhotoModeComponent* self)
		{
			self->mDCMoveDistanceLimit = UE_BIG_NUMBER;
			self->mDCCutoffDistanceLimit = UE_BIG_NUMBER;
		});
#endif
}

void FPhotoModeUntetheredModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPhotoModeUntetheredModule, PhotoModeUntethered)