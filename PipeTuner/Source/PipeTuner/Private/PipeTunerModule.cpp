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

void bphook(FBlueprintHookHelper& helper)
{
		UObject* ctx = helper.GetContext(); // the object this function got called onto
		// do some nice stuff there
		auto outer = ctx->GetOuter();
		helper.JumpToFunctionReturn();
		//helper.JumpToFunctionReturn();
}

void GameModePostLogin(CallScope<void(*)(AFGGameMode*, APlayerController*)>& scope, AFGGameMode* gm,
	APlayerController* pc)
{
#if !WITH_EDITOR

#endif
	//UClass* SomeClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/-Shared/Holo_PipelinePump.Holo_PipelinePump_C"));
	//UFunction* SomeFunc = SomeClass->FindFunctionByName(TEXT("UpdateOrAddInstances"));
	//UBlueprintHookManager* HookManager = GEngine->GetEngineSubsystem<UBlueprintHookManager>();
	//HookManager->HookBlueprintFunction(SomeFunc, &bphook, EPredefinedHookOffset::Return);
}

void FPipeTunerModule::StartupModule() {

#if !WITH_EDITOR
	AFGBuildablePipeline* plCDO = GetMutableDefault<AFGBuildablePipeline>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildablePipeline::BeginPlay, plCDO, [=](AFGBuildablePipeline* self)
		{

			auto ModConfig = FPipeTuner_ConfigStruct::GetActiveConfig();
			float mk2mult = ModConfig.Mk2PipeVolumeMultiplier;
			float mk1mult = ModConfig.Mk1PipeVolumeMultiplier;
			FFluidBox::OVERFILL_USED_FOR_PRESSURE_PCT = ModConfig.OverfillPressurePercent;
			FFluidBox::PRESSURE_LOSS = ModConfig.PressureLossPercent;
			//self->mFlowLimit = self->mFlowLimit / FluidAmount;
			//self->mFluidBox.MaxContent = self->mFluidBox.MaxContent / FluidAmount;
			self->mFluidBox.MaxOverfillPct = ModConfig.OverfillPercent;

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

	AFGGameMode* LocalGameMode = GetMutableDefault<AFGGameMode>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGGameMode::PostLogin, LocalGameMode, &GameModePostLogin)


	AFGPipelineAttachmentHologram* pah = GetMutableDefault<AFGPipelineAttachmentHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGPipelineAttachmentHologram::BeginPlay, pah, [=](auto scope, AFGPipelineAttachmentHologram* self)
		{
			auto pumphg = Cast< AFGPipelinePumpHologram>(self);
			if (pumphg)
			{
				auto ModConfig = FPipeTuner_ConfigStruct::GetActiveConfig();
				pumphg->mMaxTraversalDistance = ModConfig.PumpTraversalDistance * 100;
				pumphg->mMaxJunctionRecursions = ModConfig.PumpJunctionRecursions;
				pumphg->mOffsetEstimationBinaryDivisionCount = ModConfig.PumpOffsetEstimationCount;
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