#include "ConstructionPreferencesModule.h"
#include "Patching/NativeHookManager.h"
#include "Hologram/FGConveyorBeltHologram.h"
#include "Subsystem/SubsystemActorManager.h"
#include "Hologram/FGHologram.h"
#include "Hologram/FGPoleHologram.h"
#include "Hologram/FGConveyorPoleHologram.h"
#include "Equipment/FGBuildGunBuild.h"
#include "Hologram/FGWireHologram.h"
#include "Holo_WireHologramBuildModes.h"
//#include "Buildables/FGConveyorPoleStackable.h"
#include "Equipment/FGBuildGunDismantle.h"
#include "Equipment/FGHoverPack.h"
#include "CP_ModConfigStruct.h"
#include "FGCharacterPlayer.h"
#include "Hologram/FGConveyorLiftHologram.h"
#include "Hologram/FGConveyorPoleHologram.h"
#include "CP_Subsystem.h"
#include "FGBuildableSubsystem.h"
#include "FGCharacterPlayer.h"
#include "Equipment/FGBuildGun.h"
#include "Equipment/FGParachute.h"
#include "Hologram/FGBlueprintHologram.h"
#include <SessionSettings/SessionSettingsManager.h>
#include <Logging/StructuredLog.h>

DEFINE_LOG_CATEGORY(LogConstructionPreferences);

//#pragma optimize("", off)
float FConstructionPreferencesModule::GetUseDistance(const AFGCharacterPlayer* self, float defaultDistance)
{
	USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
	auto reachMult = SessionSettings->GetFloatOptionValue("ConstructionPreferences.UseDistance");
	return reachMult * defaultDistance;
}
//#pragma optimize("", on)

void FConstructionPreferencesModule::StartupModule() {
#if !WITH_EDITOR
	SUBSCRIBE_METHOD(AFGCharacterPlayer::GetUseDistance, [this](auto& scope, const AFGCharacterPlayer* self)
		{
			float result = scope(self);
			float newDist = 0.f;
			newDist = GetUseDistance(self, result);
			if (newDist > 0)
			{
				scope.Override(newDist);
			}
		});

	SUBSCRIBE_METHOD(AFGBuildGun::GetBuildGunRange, [this](auto& scope, const AFGBuildGun* self)
		{
			USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			auto reachMult = SessionSettings->GetFloatOptionValue("ConstructionPreferences.UseDistance");

			auto bg = const_cast<AFGBuildGun*>(self);
			auto defaultRange = bg->GetDefaultBuildGunRange();

			scope.Override(bg->mBuildDistanceMax * reachMult);
		});

	AFGConveyorLiftHologram* hg = GetMutableDefault<AFGConveyorLiftHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGConveyorLiftHologram::BeginPlay, hg, [](AFGConveyorLiftHologram* self)
		{
			USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			auto stepHeight = SessionSettings->GetFloatOptionValue("ConstructionPreferences.ConveyorLift.StepHeight");
			auto minHeight = SessionSettings->GetFloatOptionValue("ConstructionPreferences.ConveyorLift.MinHeight");
			auto maxHeight = SessionSettings->GetFloatOptionValue("ConstructionPreferences.ConveyorLift.MaxHeight");

			self->mStepHeight = (stepHeight * 100);
			self->mMinimumHeight = (minHeight * 100);
			self->mMaximumHeight = (maxHeight * 100);
		});

	AFGConveyorBeltHologram* bhg = GetMutableDefault<AFGConveyorBeltHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGConveyorBeltHologram::BeginPlay, bhg, [](AFGConveyorBeltHologram* self)
		{
			USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			auto length = SessionSettings->GetFloatOptionValue("ConstructionPreferences.ConveyorBelt.Length");
			if (!FMath::IsNearlyEqual(length, 56, 0.1)) // Check if default
			{
				self->mMaxSplineLength = length * 100;
			}
		});

	AFGPipelineHologram* phg = GetMutableDefault<AFGPipelineHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGPipelineHologram::BeginPlay, phg, [](AFGPipelineHologram* self)
		{
			// Determine if is pipe or hypertube
			USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			if (self->mBuildClass->GetName().ToLower().Contains("hyper"))
			{
				// Do hypertube stuff
				//UE_LOGFMT(LogConstructionPreferences, Display, "Hypertube Max Length: {0}", self->mMaxSplineLength);
				auto length = SessionSettings->GetFloatOptionValue("ConstructionPreferences.Hypertube.Length");
				if (!FMath::IsNearlyEqual(length, 100, 0.1)) // Check if default
				{
					self->mMaxSplineLength = length * 100;
				}
			}
			else
			{
				auto length = SessionSettings->GetFloatOptionValue("ConstructionPreferences.Pipeline.Length");
				if (!FMath::IsNearlyEqual(length, 56, 0.1)) // Check if default
				{
					self->mMaxSplineLength = length * 100;
				}
			}
		});

	AFGRailroadTrackHologram* rrhg = GetMutableDefault<AFGRailroadTrackHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGRailroadTrackHologram::BeginPlay, rrhg, [](AFGRailroadTrackHologram* self)
		{
			USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			auto length = SessionSettings->GetFloatOptionValue("ConstructionPreferences.Railroad.Length");
			if (!FMath::IsNearlyEqual(length, 100, 0.1)) // Check if default
			{
				self->mMaxLength = length * 100;
				//UE_LOGFMT(LogConstructionPreferences, Display, "Railroad Max Length: {0}", self->mMaxLength);
			}
		});

	AFGWireHologram* whg = GetMutableDefault<AFGWireHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGWireHologram::BeginPlay, whg, [](AFGWireHologram* self)
		{
			USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			auto length = SessionSettings->GetFloatOptionValue("ConstructionPreferences.Wire.Length");
			//UE_LOGFMT(LogConstructionPreferences, Display, "Wire Length: {0}", length);
			if (!FMath::IsNearlyEqual(length, 100, 0.1)) // Check if default
			{
				self->mMaxLength = length * 100;
			}

			auto towerLength = SessionSettings->GetFloatOptionValue("ConstructionPreferences.TowerWire.Length");
			//UE_LOGFMT(LogConstructionPreferences, Display, "Tower Wire Max Length: {0}", self->mMaxPowerTowerLength);
			if (!FMath::IsNearlyEqual(towerLength, 300, 0.1)) // Check if default
			{
				self->mMaxPowerTowerLength = towerLength * 100;
			}
		});
#endif
}

IMPLEMENT_GAME_MODULE(FConstructionPreferencesModule, ConstructionPreferences);