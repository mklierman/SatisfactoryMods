// Copyright Epic Games, Inc. All Rights Reserved.

#include "IdenticalBuildable.h"
#include "Patching/NativeHookManager.h"
#include "Hologram/FGBuildableHologram.h"
#include "Hologram/FGBeamHologram.h"
#include "Hologram/FGFoundationHologram.h"
#include "Hologram/FGGenericBuildableHologram.h"
#include "Hologram/FGPillarHologram.h"
#include "Hologram/FGPipelineSupportHologram.h"
#include "Hologram/FGStairHologram.h"
#include "Hologram/FGWallHologram.h"
#include "Hologram/FGRampHologram.h"

#define LOCTEXT_NAMESPACE "FIdenticalBuildableModule"

void FIdenticalBuildableModule::StartupModule()
{
	AFGBuildableHologram* BuildableHologramCDO = GetMutableDefault<AFGBuildableHologram>();
	AFGBeamHologram* BeamHologramCDO = GetMutableDefault<AFGBeamHologram>();
	AFGFoundationHologram* FoundationHologramCDO = GetMutableDefault<AFGFoundationHologram>();
	AFGGenericBuildableHologram* GenericBuildableHologramCDO = GetMutableDefault<AFGGenericBuildableHologram>();
	AFGPillarHologram* PillarHologramCDO = GetMutableDefault<AFGPillarHologram>();
	AFGPipelineSupportHologram* PipelineSupportHologramCDO = GetMutableDefault<AFGPipelineSupportHologram>();
	AFGStairHologram* StairHologramCDO = GetMutableDefault<AFGStairHologram>();
	AFGWallHologram* WallHologramCDO = GetMutableDefault<AFGWallHologram>();
	AFGRampHologram* RampHologramCDO = GetMutableDefault<AFGRampHologram>();

	SUBSCRIBE_METHOD_VIRTUAL(AFGBuildableHologram::IsHologramIdenticalToActor, BuildableHologramCDO, [](auto& scope,const AFGBuildableHologram* self, AActor* actor, const FVector& hologramLocationOffset)
		{
			if (self && actor)
			{
				scope.Override(false);
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGBeamHologram::IsHologramIdenticalToActor, BeamHologramCDO, [](auto& scope, const AFGBeamHologram* self, AActor* actor, const FVector& hologramLocationOffset)
		{
			if (self && actor)
			{
				scope.Override(false);
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGFoundationHologram::IsHologramIdenticalToActor, FoundationHologramCDO, [](auto& scope, const AFGFoundationHologram* self, AActor* actor, const FVector& hologramLocationOffset)
		{
			if (self && actor)
			{
				scope.Override(false);
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGGenericBuildableHologram::IsHologramIdenticalToActor, GenericBuildableHologramCDO, [](auto& scope, const AFGGenericBuildableHologram* self, AActor* actor, const FVector& hologramLocationOffset)
		{
			if (self && actor)
			{
				scope.Override(false);
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGPillarHologram::IsHologramIdenticalToActor, PillarHologramCDO, [](auto& scope, const AFGPillarHologram* self, AActor* actor, const FVector& hologramLocationOffset)
		{
			if (self && actor)
			{
				scope.Override(false);
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGPipelineSupportHologram::IsHologramIdenticalToActor, PipelineSupportHologramCDO, [](auto& scope, const AFGPipelineSupportHologram* self, AActor* actor, const FVector& hologramLocationOffset)
		{
			if (self && actor)
			{
				scope.Override(false);
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGStairHologram::IsHologramIdenticalToActor, StairHologramCDO, [](auto& scope, const AFGStairHologram* self, AActor* actor, const FVector& hologramLocationOffset)
		{
			if (self && actor)
			{
				scope.Override(false);
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGWallHologram::IsHologramIdenticalToActor, WallHologramCDO, [](auto& scope, const AFGWallHologram* self, AActor* actor, const FVector& hologramLocationOffset)
		{
			if (self && actor)
			{
				scope.Override(false);
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGRampHologram::IsHologramIdenticalToActor, RampHologramCDO, [](auto& scope, const AFGRampHologram* self, AActor* actor, const FVector& hologramLocationOffset)
		{
			if (self && actor)
			{
				scope.Override(false);
			}
		});
}

void FIdenticalBuildableModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FIdenticalBuildableModule, IdenticalBuildable)