#include "ConstructionPreferencesModule.h"
#include "Patching/NativeHookManager.h"
#include "Hologram/FGConveyorBeltHologram.h"
#include "Subsystem/SubsystemActorManager.h"
#include "Hologram/FGHologram.h"
#include "Hologram/FGPoleHologram.h"
#include "Equipment/FGBuildGunBuild.h"
#include "CP_ModConfigStruct.h"
#include "Hologram/FGConveyorLiftHologram.h"
#include "CP_Subsystem.h"

DEFINE_LOG_CATEGORY(LogConstructionPreferences);
void FConstructionPreferencesModule::StartupModule() {

#if !WITH_EDITOR
	AFGConveyorLiftHologram* hg = GetMutableDefault<AFGConveyorLiftHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGConveyorLiftHologram::BeginPlay, hg, [](AFGConveyorLiftHologram* self)
		{
			FCP_ModConfigStruct config = FCP_ModConfigStruct::GetActiveConfig();
			
			self->mStepHeight = ((float)config.ConveyorLiftStep * 100);
			self->mMinimumHeight = ((float)config.ConveyorLiftHeight * 100);
		});
	//AFGConveyorBeltHologram* CBH = GetMutableDefault<AFGConveyorBeltHologram>();
//	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorBeltHologram::SpawnChildren, CBH, [](auto scope, AFGConveyorBeltHologram* self, AActor* hologramOwner, FVector spawnLocation, APawn* hologramInstigator) {
//UE_LOG(LogConstructionPreferences, Display, TEXT("AFGConveyorBeltHologram::SpawnChildren"));
//		//UWorld* WorldObject = self->GetWorld();
//		//USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
//		//ACP_Subsystem* subsystem = SubsystemActorManager->GetSubsystemActor<ACP_Subsystem>();
//		//auto recipe = subsystem->mConveyorPoleRecipe;
//		//self->SpawnChildHologramFromRecipe(self, recipe, hologramOwner, spawnLocation, hologramInstigator);
//		//scope.Cancel();
//		});
//
//
//	//virtual bool DoMultiStepPlacement(bool isInputFromARelease) override;
//	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorBeltHologram::DoMultiStepPlacement, CBH, [](auto scope, AFGConveyorBeltHologram* self, bool isInputFromARelease)
//		{
//			UE_LOG(LogConstructionPreferences, Display, TEXT("AFGConveyorBeltHologram::DoMultiStepPlacement"));
//			//FDebug::DumpStackTraceToLog(ELogVerbosity::Display);
//			scope.Override(false);
//		});
//
//	AFGPoleHologram* CPH = GetMutableDefault<AFGPoleHologram>();
//	SUBSCRIBE_METHOD_VIRTUAL(AFGPoleHologram::DoMultiStepPlacement, CPH, [](auto scope, AFGPoleHologram* self, bool isInputFromARelease)
//		{
//			UE_LOG(LogConstructionPreferences, Display, TEXT("AFGPoleHologram::DoMultiStepPlacement"));
//			//FDebug::DumpStackTraceToLog(ELogVerbosity::Display);
//			scope.Override(false);
//		});
//	//virtual void GetSupportedScrollModes(TArray<EHologramScrollMode>*out_modes) const override;
//	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorBeltHologram::GetSupportedScrollModes, CBH, [](auto scope, const AFGConveyorBeltHologram* self, TArray<EHologramScrollMode>* out_modes)
//		{
//			UE_LOG(LogConstructionPreferences, Display, TEXT("AFGConveyorBeltHologram::GetSupportedScrollModes"));
//			out_modes->Empty();
//			out_modes->Add(EHologramScrollMode::HSM_ROTATE);
//			scope.Cancel();
//		});
//
//	//	virtual void SetHologramLocationAndRotation(const FHitResult & hitResult) override;
//	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorBeltHologram::SetHologramLocationAndRotation, CBH, [](auto scope, AFGConveyorBeltHologram* self, const FHitResult& hitResult)
//		{
//			//UE_LOG(LogConstructionPreferences, Display, TEXT("AFGConveyorBeltHologram::SetHologramLocationAndRotation"));
//		});
//
//	//	virtual bool IsValidHitResult(const FHitResult & hitResult) const override;
//	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorBeltHologram::IsValidHitResult, CBH, [](auto scope, const AFGConveyorBeltHologram* self, const FHitResult& hitResult)
//		{
//			//UE_LOG(LogConstructionPreferences, Display, TEXT("AFGConveyorBeltHologram::IsValidHitResult"));
//		});
//
//	//	virtual bool TrySnapToActor(const FHitResult & hitResult) override;
//	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorBeltHologram::TrySnapToActor, CBH, [](auto scope, AFGConveyorBeltHologram* self, const FHitResult& hitResult)
//		{
//			//UE_LOG(LogConstructionPreferences, Display, TEXT("AFGConveyorBeltHologram::TrySnapToActor"));
//		});
//
//	//	virtual void CheckValidPlacement() override;
//	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorBeltHologram::CheckValidPlacement, CBH, [](auto scope, AFGConveyorBeltHologram* self)
//		{
//			//UE_LOG(LogConstructionPreferences, Display, TEXT("AFGConveyorBeltHologram::CheckValidPlacement"));
//		});
//
//	//bool CanConstruct() const;
//	SUBSCRIBE_METHOD(AFGHologram::CanConstruct, [](auto scope, const AFGHologram* self)
//		{
//			//auto bString = returnValue ? "true" : "false";
//			//UE_LOG(LogConstructionPreferences, Display, TEXT("AFGHologram::CanConstruct - %s"), bString);
//			scope.Override(true);
//		});

	//virtual bool CanTakeNextBuildStep() const;
	//SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::CanTakeNextBuildStep, CBH, [](auto scope, const AFGHologram* self)
	//	{
	//		//auto bString = returnValue ? "true" : "false";
	//		//UE_LOG(LogConstructionPreferences, Display, TEXT("AFGHologram::CanTakeNextBuildStep - %s"), bString);
	//		scope.Override(true);
	//	});

	//UFGBuildGunStateBuild* BG = GetMutableDefault<UFGBuildGunStateBuild>();
	//virtual void PrimaryFire_Implementation() override;
	//virtual void PrimaryFireRelease_Implementation() override;
	//SUBSCRIBE_METHOD_VIRTUAL_AFTER(UFGBuildGunStateBuild::PrimaryFire_Implementation, BG, [](UFGBuildGunStateBuild* self)
	//	{
	//		UE_LOG(LogConstructionPreferences, Display, TEXT("UFGBuildGunStateBuild::PrimaryFire_Implementation"));
	//		FDebug::DumpStackTraceToLog(ELogVerbosity::Display);
	//	});

	//SUBSCRIBE_METHOD_VIRTUAL_AFTER(UFGBuildGunStateBuild::PrimaryFireRelease_Implementation, BG, [](UFGBuildGunStateBuild* self)
	//	{
	//		UE_LOG(LogConstructionPreferences, Display, TEXT("UFGBuildGunStateBuild::PrimaryFireRelease_Implementatio"));
	//		FDebug::DumpStackTraceToLog(ELogVerbosity::Display);
	//	});


	//FORCEINLINE ESplineHologramBuildStep GetCurrentBuildStep() { return mBuildStep; }
	//SUBSCRIBE_METHOD(AFGSplineHologram::GetCurrentBuildStep, [](auto scope, const AFGSplineHologram* self)
	//	{
	//		//auto bString = returnValue ? "true" : "false";
	//		//UE_LOG(LogConstructionPreferences, Display, TEXT("AFGHologram::CanTakeNextBuildStep - %s"), bString);
	//		if (self->mBuildStep == ESplineHologramBuildStep::SHBS_PlacePoleOrSnapEnding)
	//		{
	//			self->mBuildStep = ESplineHologramBuildStep::SHBS_AdjustPole;
	//			scope.Override(ESplineHologramBuildStep::SHBS_AdjustPole);
	//		}
	//		//scope.Override(true);
	//	});
#endif
}


IMPLEMENT_GAME_MODULE(FConstructionPreferencesModule, ConstructionPreferences);