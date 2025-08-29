#include "PipeTunerModule.h"
#include "FGFluidIntegrantInterface.h"
#include "PipeTuner_ConfigStruct.h"
#include "CoreMinimal.h"
#include "Patching/BlueprintHookManager.h"
#include "Buildables/FGBuildablePipeline.h"
#include "FGGameMode.h"
#include "Hologram/FGPipelinePumpHologram.h"
#include "Hologram/FGPipelineAttachmentHologram.h"
#include <SessionSettings/SessionSettingsManager.h>
#include "FGPipeSubsystem.h"
#include <Subsystem/SubsystemActorManager.h>
#include "PipeTuner_Subsystem.h"
#include "FGPipeNetwork.h"

void FPipeTunerModule::StartupModule() {

#if !WITH_EDITOR
	AFGBuildablePipeline* plCDO = GetMutableDefault<AFGBuildablePipeline>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildablePipeline::BeginPlay, plCDO, [=](AFGBuildablePipeline* self)
		{
			self->GetWorld();
			USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			if (!SessionSettings)
			{
				return;
			}
			auto Mk1VolumeMultiplier = SessionSettings->GetFloatOptionValue("PipeTuner.Mk1VolumeMultiplier");
			auto Mk2VolumeMultiplier = SessionSettings->GetFloatOptionValue("PipeTuner.Mk2VolumeMultiplier");
			auto OverfillPercent = SessionSettings->GetFloatOptionValue("PipeTuner.OverfillPercent");
			auto OverfillPressurePercent = SessionSettings->GetFloatOptionValue("PipeTuner.OverfillPressurePercent");
			auto PressureLossPercent = SessionSettings->GetFloatOptionValue("PipeTuner.PressureLossPercent");

			float mk2mult = Mk2VolumeMultiplier;
			float mk1mult = Mk1VolumeMultiplier;
			FFluidBox::OVERFILL_USED_FOR_PRESSURE_PCT = OverfillPressurePercent / 100.0;
			FFluidBox::PRESSURE_LOSS = PressureLossPercent / 100.0;
			self->mFluidBox.MaxOverfillPct = OverfillPercent / 100.0;

			USubsystemActorManager* SubsystemActorManager = self->GetWorld()->GetSubsystem<USubsystemActorManager>();
			APipeTuner_Subsystem* tunerSubsystem = SubsystemActorManager->GetSubsystemActor<APipeTuner_Subsystem>();
			if (!tunerSubsystem)
				return;
			tunerSubsystem->PipeVolumes.Add(self, self->mFluidBox.MaxContent);
			auto name = self->GetName();
			if (name.Contains("MK2"))
			{
				self->mFluidBox.MaxContent = self->mFluidBox.MaxContent * mk2mult;
			}
			else
			{
				self->mFluidBox.MaxContent = self->mFluidBox.MaxContent * mk1mult;
			}
		});

	AFGPipelineAttachmentHologram* pah = GetMutableDefault<AFGPipelineAttachmentHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGPipelineAttachmentHologram::BeginPlay, pah, [=](auto scope, AFGPipelineAttachmentHologram* self)
		{
			auto pumphg = Cast< AFGPipelinePumpHologram>(self);
			if (pumphg)
			{
				USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
				if (!SessionSettings)
				{
					return;
				}
				auto MaxTraversalDistance = SessionSettings->GetFloatOptionValue("PipeTuner.PumpTraversalDistance");
				auto PumpJunctionRecursions = (int32)SessionSettings->GetFloatOptionValue("PipeTuner.JunctionRecursions");
				auto PumpOffsetEstimationCount = (int32)SessionSettings->GetFloatOptionValue("PipeTuner.PumpOffsetEstimationCount");

				pumphg->mMaxTraversalDistance = MaxTraversalDistance * 100;
				pumphg->mMaxJunctionRecursions = PumpJunctionRecursions;
				pumphg->mOffsetEstimationBinaryDivisionCount = PumpOffsetEstimationCount;
			}
		});

	SUBSCRIBE_METHOD_AFTER(AFGPipeSubsystem::RegisterPipeNetwork, [](AFGPipeSubsystem* self, AFGPipeNetwork* network) {
		USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
		if (!SessionSettings)
		{
			return;
		}
		auto Gravity = SessionSettings->GetFloatOptionValue("PipeTuner.Gravity");
		auto Friction = SessionSettings->GetFloatOptionValue("PipeTuner.Friction");
		auto Density = SessionSettings->GetFloatOptionValue("PipeTuner.Density");
		auto Viscosity = SessionSettings->GetFloatOptionValue("PipeTuner.Viscosity");

		network->mGravity = Gravity;
		network->mFluidFriction = Friction;
		network->mFluidDensity = Density;
		network->mFluidViscosity = Viscosity;
		});

#endif
	//AFGPipelinePumpHologram* pdh = GetMutableDefault<AFGPipelinePumpHologram>();
}

IMPLEMENT_GAME_MODULE(FPipeTunerModule, PipeTuner);