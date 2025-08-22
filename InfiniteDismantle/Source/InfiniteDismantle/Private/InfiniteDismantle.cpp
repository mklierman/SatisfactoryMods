// Copyright Epic Games, Inc. All Rights Reserved.

#include "InfiniteDismantle.h"
#include "Equipment/FGBuildGun.h"
#include "Patching/NativeHookManager.h"
#include "Equipment/FGBuildGunDismantle.h"

#define LOCTEXT_NAMESPACE "FInfiniteDismantleModule"

void FInfiniteDismantleModule::StartupModule()
{
	UFGBuildGunStateDismantle* bgsd = GetMutableDefault<UFGBuildGunStateDismantle>();
	SUBSCRIBE_METHOD_VIRTUAL(UFGBuildGunStateDismantle::TickState_Implementation, bgsd, [](auto& scope, UFGBuildGunStateDismantle* self, float deltaTime)
		{
			self->mCurrentMultiDismantleLimit = 100000;
		});
}

void FInfiniteDismantleModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInfiniteDismantleModule, InfiniteDismantle)