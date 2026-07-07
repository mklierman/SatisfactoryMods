// Copyright Epic Games, Inc. All Rights Reserved.

#include "GrabAllFromBelt.h"
#include "FGItemPickup.h"
#include "FGConveyorChainActor.h"
#include "Patching/NativeHookManager.h"
#include "FGCharacterPlayer.h"

#define LOCTEXT_NAMESPACE "FGrabAllFromBeltModule"

DEFINE_LOG_CATEGORY(GrabAllFromBelt_Log);
void FGrabAllFromBeltModule::StartupModule()
{
}

void FGrabAllFromBeltModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGrabAllFromBeltModule, GrabAllFromBelt)