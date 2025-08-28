


#include "PipeTuner_BPFL.h"
#include <SessionSettings/SessionSettingsManager.h>
#include "FGPipeSubsystem.h"
#include "FGPipeNetwork.h"
#include "Buildables/FGBuildablePipeline.h"
#include <Kismet/GameplayStatics.h>

void UPipeTuner_BPFL::UpdateValues(UGameInstance* instance)
{
	auto world = instance->GetWorld();
	TArray<AActor*> out_Actors;
	UGameplayStatics::GetAllActorsOfClass(world, AFGBuildablePipeline::StaticClass(), out_Actors);

	USessionSettingsManager* SessionSettings = world->GetSubsystem<USessionSettingsManager>();
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
	for (auto actor : out_Actors)
	{
		auto pipeline = Cast< AFGBuildablePipeline>(actor);
		if (!pipeline)
		{
			continue;
		}
		pipeline->mFluidBox.MaxOverfillPct = OverfillPercent / 100.0;

		auto name = pipeline->GetName();
		if (name.Contains("MK2"))
		{
			pipeline->mFluidBox.MaxContent = 5.0 * mk2mult;
		}
		else
		{
			pipeline->mFluidBox.MaxContent = 5.0 * mk1mult;
		}
	}

	auto pipeSubsystem = AFGPipeSubsystem::Get(world);
	if (pipeSubsystem)
	{
		auto Gravity = SessionSettings->GetFloatOptionValue("PipeTuner.Gravity");
		auto Friction = SessionSettings->GetFloatOptionValue("PipeTuner.Friction");
		auto Density = SessionSettings->GetFloatOptionValue("PipeTuner.Density");
		auto Viscosity = SessionSettings->GetFloatOptionValue("PipeTuner.Viscosity");

		for (auto pair : pipeSubsystem->mNetworks)
		{
			AFGPipeNetwork* network = pair.Value;
			network->mGravity = Gravity;
			network->mFluidFriction = Friction;
			network->mFluidDensity = Density;
			network->mFluidViscosity = Viscosity;
		}
	}
}
