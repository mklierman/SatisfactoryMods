// Copyright Epic Games, Inc. All Rights Reserved.

#include "AlwaysRelevantSigns.h"
#include "FGSignSubsystem.h"
#include "Patching/NativeHookManager.h"

#define LOCTEXT_NAMESPACE "FAlwaysRelevantSignsModule"

void FAlwaysRelevantSignsModule::StartupModule()
{
#if !WITH_EDITOR
	SUBSCRIBE_METHOD(AFGSignSubsystem::UpdateRelevancy, [this](auto& scope, AFGSignSubsystem* self)
		{
			scope.Cancel();
		});
#endif
}

void FAlwaysRelevantSignsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAlwaysRelevantSignsModule, AlwaysRelevantSigns)