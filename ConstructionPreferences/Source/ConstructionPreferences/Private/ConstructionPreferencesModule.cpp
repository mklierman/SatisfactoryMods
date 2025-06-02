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
			self->mMaxSplineLength = length * 100;
		});

	AFGPipelineHologram* phg = GetMutableDefault<AFGPipelineHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGPipelineHologram::BeginPlay, phg, [](AFGPipelineHologram* self)
		{
			USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			auto length = SessionSettings->GetFloatOptionValue("ConstructionPreferences.Pipeline.Length");
			self->mMaxSplineLength = length * 100;
		});

	AFGRailroadTrackHologram* rrhg = GetMutableDefault<AFGRailroadTrackHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGRailroadTrackHologram::BeginPlay, rrhg, [](AFGRailroadTrackHologram* self)
		{
			//UE_LOGFMT(LogConstructionPreferences, Display, "Railroad Max Length: {0}", self->mMaxLength);
			USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			auto length = SessionSettings->GetFloatOptionValue("ConstructionPreferences.Railroad.Length");
			self->mMaxLength = length * 100;
		});
#endif

}

IMPLEMENT_GAME_MODULE(FConstructionPreferencesModule, ConstructionPreferences);