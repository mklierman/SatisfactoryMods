#include "PipeTunerModule.h"
#include "FGFluidIntegrantInterface.h"
#include "PipeTuner_ConfigStruct.h"
#include "CoreMinimal.h"
#include "Patching/BlueprintHookManager.h"
#include "Patching/BlueprintHookHelper.h"
#include "Buildables/FGBuildablePipeline.h"
#include "FGGameMode.h"
#include "Hologram/FGPipelinePumpHologram.h"
#include "Hologram/FGPipelineAttachmentHologram.h"
#include <Logging/StructuredLog.h>
#include <FGPipeNetwork.h>

DEFINE_LOG_CATEGORY(LogPipeTuner);

void bphook(FBlueprintHookHelper& helper)
{
		UObject* ctx = helper.GetContext(); // the object this function got called onto
		// do some nice stuff there
		auto outer = ctx->GetOuter();
		helper.JumpToFunctionReturn();
		//helper.JumpToFunctionReturn();
}


void FPipeTunerModule::StartupModule() {

#if !WITH_EDITOR
	AFGPipeNetwork* pnCDO = GetMutableDefault<AFGPipeNetwork>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGPipeNetwork::BeginPlay,pnCDO, [this](AFGPipeNetwork* self)
		{
			UE_LOGFMT(LogPipeTuner, Display, "mGravity: {0}", self->mGravity);
			UE_LOGFMT(LogPipeTuner, Display, "mFluidFriction: {0}", self->mFluidFriction);
			UE_LOGFMT(LogPipeTuner, Display, "mFluidDensity: {0}", self->mFluidDensity);
			UE_LOGFMT(LogPipeTuner, Display, "mFluidViscosity: {0}", self->mFluidViscosity);
		});

	AFGBuildablePipeline* plCDO = GetMutableDefault<AFGBuildablePipeline>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildablePipeline::BeginPlay, plCDO, [this](AFGBuildablePipeline* self)
		{

			FPipeTuner_ConfigStruct ModConfig = FPipeTuner_ConfigStruct::GetActiveConfig(self->GetWorld());
			float mk2mult = ModConfig.Mk2PipeVolumeMultiplier;
			float mk1mult = ModConfig.Mk1PipeVolumeMultiplier;
			UE_LOGFMT(LogPipeTuner, Display, "OVERFILL_USED_FOR_PRESSURE_PCT: {0}", FFluidBox::OVERFILL_USED_FOR_PRESSURE_PCT);
			UE_LOGFMT(LogPipeTuner, Display, "PRESSURE_LOSS: {0}", FFluidBox::PRESSURE_LOSS);
			//FFluidBox::OVERFILL_USED_FOR_PRESSURE_PCT = ModConfig.OverfillPressurePercent;
			//FFluidBox::PRESSURE_LOSS = ModConfig.PressureLossPercent;
			//self->mFlowLimit = self->mFlowLimit / FluidAmount;
			//self->mFluidBox.MaxContent = self->mFluidBox.MaxContent / FluidAmount;
			//self->mFluidBox.MaxOverfillPct = ModConfig.OverfillPercent;
			UE_LOGFMT(LogPipeTuner, Display, "mFluidBox.MaxOverfillPct: {0}", self->mFluidBox.MaxOverfillPct);

			auto name = self->GetName();
			if (name.Contains("MK2"))
			{
				UE_LOGFMT(LogPipeTuner, Display, "mFluidBoxMaxContent: {0}", self->mFluidBox.MaxContent);
				//self->mFluidBox.MaxContent = self->mFluidBox.MaxContent * mk2mult;
			}
			else
			{
				UE_LOGFMT(LogPipeTuner, Display, "mFluidBoxMaxContent: {0}", self->mFluidBox.MaxContent);
				//self->mFluidBox.MaxContent = self->mFluidBox.MaxContent * mk1mult;
			}
		});



	AFGPipelineAttachmentHologram* pah = GetMutableDefault<AFGPipelineAttachmentHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGPipelineAttachmentHologram::BeginPlay, pah, [this](auto scope, AFGPipelineAttachmentHologram* self)
		{
			auto pumphg = Cast< AFGPipelinePumpHologram>(self);
			if (pumphg)
			{
				UE_LOGFMT(LogPipeTuner, Display, "mMaxTraversalDistance: {0}", pumphg->mMaxTraversalDistance);
				UE_LOGFMT(LogPipeTuner, Display, "mMaxJunctionRecursions: {0}", pumphg->mMaxJunctionRecursions);
				UE_LOGFMT(LogPipeTuner, Display, "mOffsetEstimationBinaryDivisionCount: {0}", pumphg->mOffsetEstimationBinaryDivisionCount);
				FPipeTuner_ConfigStruct ModConfig = FPipeTuner_ConfigStruct::GetActiveConfig(self->GetWorld());
				//pumphg->mMaxTraversalDistance = ModConfig.PumpTraversalDistance * 100;
				//pumphg->mMaxJunctionRecursions = ModConfig.PumpJunctionRecursions;
				//pumphg->mOffsetEstimationBinaryDivisionCount = ModConfig.PumpOffsetEstimationCount;
				//if (ActiveHologramTimers.Find(pumphg) == nullptr)
				//{
				//	ActiveHologramTimers.Add(pumphg, PumpTimer);
				//	FTimerHandle PumpTimer;
				//	GetWorld()->GetTimerManager().SetTimer(PumpTimer, self, &FPipeTunerModule::SpawnPumpHG, 5.f, true, 5.f);
				//}
			}
		});

#endif
	//AFGPipelinePumpHologram* pdh = GetMutableDefault<AFGPipelinePumpHologram>();
}

void FPipeTunerModule::SpawnPumpHG()
{
	TArray< AFGPipelinePumpHologram*> hgs;
	ActiveHologramTimers.GetKeys(hgs);
	if (hgs.Num() > 0)
	{
		for (auto hg : hgs)
		{
			if (hg)
			{
				//FPumpHeadLiftLocationPath newPath;
				FPumpHeadLiftLocationPath currentPath = hg->mCurrentHeadLiftPath;
				//newPath.SetData(currentPath.Spline, currentPath.OffsetStart, currentPath.OffsetEnd, currentPath.ReverseSplineDirection);
				const class USplineComponent* Spline = currentPath.Spline;
				float OffsetStart = currentPath.OffsetStart;
				float OffsetEnd = currentPath.OffsetEnd;
				bool ReverseSplineDirection = currentPath.ReverseSplineDirection;
				currentPath.AddNextPath(Spline, OffsetStart, OffsetEnd, ReverseSplineDirection);
			}
			else
			{
				//MyTimerManager.ClearTimer(ActiveHologramTimers[hg]);
				ActiveHologramTimers.Remove(hg);
			}
		}
	}
}


IMPLEMENT_GAME_MODULE(FPipeTunerModule, PipeTuner);