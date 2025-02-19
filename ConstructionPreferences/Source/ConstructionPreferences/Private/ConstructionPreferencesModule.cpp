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
#include "Buildables/FGConveyorPoleStackable.h"
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

DEFINE_LOG_CATEGORY(LogConstructionPreferences);

//void PrimaryFire(CallScope<void(*)(UFGBuildGunStateBuild*)>& scope, UFGBuildGunStateBuild* gm)
//{
//	UE_LOG(LogConstructionPreferences, Display, TEXT("UFGBuildGunStateBuild::PrimaryFire_Implementation"));
//
//	auto owner = Cast<AFGCharacterPlayer>(gm->GetBuildGun()->GetOwner());
//	if (owner)
//	{
//		auto locControlled = owner->IsLocallyControlled();
//		auto upgActor = gm->mUpgradedActor;
//		auto mHologram = gm->mHologram;
//		if (mHologram)
//		{
//			auto buildGun = gm->GetBuildGun();
//			auto inventory = buildGun->GetInventory();
//			mHologram->ValidatePlacementAndCost(inventory);
//			if (mHologram->CanTakeNextBuildStep())
//			{
//				auto canDoMultiStep = mHologram->DoMultiStepPlacement(false);
//				if (locControlled && canDoMultiStep)
//				{
//					UE_LOG(LogConstructionPreferences, Display, TEXT("DoMultiStep"));
//				}
//			}
//		}
//	}
//}

//#pragma optimize("", off)
float FConstructionPreferencesModule::GetUseDistance(const AFGCharacterPlayer* self, float defaultDistance)
{
	USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
	auto reachMult = SessionSettings->GetFloatOptionValue("ConstructionPreferences.UseDistance");
	return reachMult * defaultDistance;

	//if (reachMult == 1.f)
	//{
	//	return 1.f * defaultDistance;
	//}

	//auto backEquip = self->mBackEquipmentSlot;
	//if (backEquip)
	//{
	//	auto hp = Cast<AFGHoverPack>(backEquip->mEquipmentInSlot);
	//	if (hp)
	//	{
	//		if (hp->mCurrentHoverMode == EHoverPackMode::HPM_Hover)
	//		{
	//			return defaultDistance + reachMult;
	//		}
	//	}
	//	auto para = Cast<AFGParachute>(backEquip->mEquipmentInSlot);
	//	if (para)
	//	{
	//		if (para->IsDeployed())
	//		{
	//			return defaultDistance + reachMult;
	//		}
	//	}
	//}
	//return reachMult ;

	//FCP_ModConfigStruct config = FCP_ModConfigStruct::GetActiveConfig(self->GetWorld());
	//float reachDist = config.ReachDistance;
	//if (reachDist <= 250.f) //250 is default
	//{
	//	return -1.f;
	//}

	//auto backEquip = self->mBackEquipmentSlot;
	//if (backEquip)
	//{
	//	auto hp = Cast<AFGHoverPack>(backEquip->mEquipmentInSlot);
	//	if (hp)
	//	{
	//		if (hp->mCurrentHoverMode == EHoverPackMode::HPM_Hover)
	//		{
	//			return 2000.0 + reachDist;
	//		}
	//	}
	//	auto para = Cast<AFGParachute>(backEquip->mEquipmentInSlot);
	//	if (para)
	//	{
	//		if (para->IsDeployed())
	//		{
	//			return 2000.0 + reachDist;
	//		}
	//	}
	//}
	//return reachDist;
}
//#pragma optimize("", on)

void FConstructionPreferencesModule::StartupModule() {

	//SUBSCRIBE_METHOD(AFGWireHologram::SetActiveAutomaticPoleHologram, [=](auto& scope, AFGWireHologram* self, class AFGPowerPoleHologram* poleHologram)
	//	{
	//		UE_LOG(LogConstructionPreferences, Display, TEXT("SetActiveAutomaticPoleHologram"));
	//		auto castSelf = Cast< AHolo_WireHologramBuildModes>(self);
	//		if (castSelf)
	//		{
	//			UE_LOG(LogConstructionPreferences, Display, TEXT("if (castSelf)"));
	//			if (castSelf->AllowSetActiveHologram)
	//			{
	//				UE_LOG(LogConstructionPreferences, Display, TEXT("if (AllowSetActiveHologram)"));
	//				return;
	//			}
	//			else
	//			{
	//				UE_LOG(LogConstructionPreferences, Display, TEXT("scope.Cancel();"));
	//				scope.Cancel();
	//			}
	//		}
	//	});
#if !WITH_EDITOR
	SUBSCRIBE_METHOD(AFGCharacterPlayer::GetUseDistance, [=](auto& scope, const AFGCharacterPlayer* self)
		{
			float result = scope(self);
			float newDist = 0.f;
			newDist = GetUseDistance(self, result);
			if (newDist > 0)
			{
				scope.Override(newDist);
			}
		});

	SUBSCRIBE_METHOD(AFGBuildGun::GetBuildGunRange, [=](auto& scope, const AFGBuildGun* self)
		{
			USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			auto reachMult = SessionSettings->GetFloatOptionValue("ConstructionPreferences.UseDistance");
			//FCP_ModConfigStruct config = FCP_ModConfigStruct::GetActiveConfig(self->GetWorld());
			//float reachDist = config.ReachDistance;
			auto bg = const_cast<AFGBuildGun*>(self);
			auto defaultRange = bg->GetDefaultBuildGunRange();
			//if (reachDist > defaultRange && reachDist > bg->mBuildDistanceMax)
			//{
			scope.Override(bg->mBuildDistanceMax * reachMult);
			//}
		});

	AFGConveyorLiftHologram* hg = GetMutableDefault<AFGConveyorLiftHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGConveyorLiftHologram::BeginPlay, hg, [](AFGConveyorLiftHologram* self)
		{
			USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			auto stepHeight = SessionSettings->GetFloatOptionValue("ConstructionPreferences.ConveyorLift.StepHeight");
			auto minHeight = SessionSettings->GetFloatOptionValue("ConstructionPreferences.ConveyorLift.MinHeight");
			auto maxHeight = SessionSettings->GetFloatOptionValue("ConstructionPreferences.ConveyorLift.MaxHeight");

			//FCP_ModConfigStruct config = FCP_ModConfigStruct::GetActiveConfig(self->GetWorld());
			self->mStepHeight = (stepHeight * 100);
			self->mMinimumHeight = (minHeight * 100);
			self->mMaximumHeight = (maxHeight * 100);
		});


#endif
	//AFGBlueprintHologram* bphg = GetMutableDefault<AFGBlueprintHologram>();
	//SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBlueprintHologram::Construct, bphg, [](AActor* returnValue, AFGBlueprintHologram* self, TArray< AActor* >& out_children, FNetConstructionID NetConstructionID)
	//	{
	//		for (auto child : out_children)
	//		{
	//			child->Destroy();
	//		}
	//	});

	//float GetBuildGunRange() const;


	AFGConveyorBeltHologram* CBH = GetMutableDefault<AFGConveyorBeltHologram>();

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
//	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorBeltHologram::DoMultiStepPlacement, CBH, [=](auto& scope, AFGConveyorBeltHologram* self, bool isInputFromARelease)
//		{
//			FCP_ModConfigStruct config = FCP_ModConfigStruct::GetActiveConfig();
//			if (config.ConveyorPole == 1)
//			{
//				UE_LOG(LogConstructionPreferences, Display, TEXT("AFGConveyorBeltHologram::DoMultiStepPlacement"));
//				//FDebug::DumpStackTraceToLog(ELogVerbosity::Display);
//				if (self->GetCurrentBuildStep() == ESplineHologramBuildStep::SHBS_AdjustPole)
//				{
//					UE_LOG(LogConstructionPreferences, Display, TEXT("ESplineHologramBuildStep::SHBS_AdjustPole"));
//					scope.Override(true);
//				}
//				else if (self->GetCurrentBuildStep() == ESplineHologramBuildStep::SHBS_PlacePoleOrSnapEnding)
//				{
//					UE_LOG(LogConstructionPreferences, Display, TEXT("ESplineHologramBuildStep::SHBS_PlacePoleOrSnapEnding"));
//					scope.Override(true);
//				}
//				else if (self->GetCurrentBuildStep() == ESplineHologramBuildStep::SHBS_FindStart)
//				{
//					UE_LOG(LogConstructionPreferences, Display, TEXT("ESplineHologramBuildStep::SHBS_FindStart"));
//				}
//			}
//		});
//
//
//	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorBeltHologram::Scroll, CBH, [=](auto& scope, AFGConveyorBeltHologram* self, int32 delta)
//		{
//			auto pole = Cast<AFGConveyorPoleHologram>(self->mChildPoleHologram);
//			if (pole)
//			{
//				if (pole->mSnappedBuilding)
//				{
//					auto snappedPole = Cast<AFGConveyorPoleStackable>(pole->mSnappedBuilding);
//					if (snappedPole)
//					{
//						pole->ScrollRotate(delta, pole->GetRotationStep());
//					}
//				}
//			}
//		});
////////
//	AFGPoleHologram* CPH = GetMutableDefault<AFGPoleHologram>();
//	SUBSCRIBE_METHOD_VIRTUAL(AFGPoleHologram::DoMultiStepPlacement, CPH, [](auto scope, AFGPoleHologram* self, bool isInputFromARelease)
//		{
//			UE_LOG(LogConstructionPreferences, Display, TEXT("AFGPoleHologram::DoMultiStepPlacement"));
//			//FDebug::DumpStackTraceToLog(ELogVerbosity::Display);
//			scope.Override(true);
//		});
//
//	SUBSCRIBE_METHOD_VIRTUAL(AFGPoleHologram::TrySnapToActor, CPH, [](auto& scope, AFGPoleHologram* self, const FHitResult& hitResult)
//		{
//			UE_LOG(LogConstructionPreferences, Display, TEXT("AFGPoleHologram::TrySnapToActor"));
//			auto convHG = Cast<AFGConveyorPoleHologram>(self);
//			if (convHG)
//			{
//				auto hitPole = Cast<AFGConveyorPoleStackable>(hitResult.Actor);
//				if (hitPole)
//				{
//					auto parentBelt = Cast<AFGConveyorBeltHologram>(self->mParent);
//					if (parentBelt)
//					{
//						int rotator = self->mSnapToGuideLines ? -1 : 1;
//						FVector startPos = parentBelt->mSnappedConnectionComponents[0]->GetConnectorLocation(false);
//						FVector startNormal = parentBelt->mSnappedConnectionComponents[0]->GetConnectorNormal();
//						FVector endPos = convHG->mSnapConnection->GetConnectorLocation(false);
//						FVector endNormal = convHG->mSnapConnection->GetConnectorNormal();
//						if (self->mSnapToGuideLines)
//						{
//							endPos = endPos.RotateAngleAxis(180, endPos);
//							endNormal = endNormal.RotateAngleAxis(180, endNormal);
//						}
//						parentBelt->AutoRouteSpline(startPos, startNormal, endPos, endNormal);
//						parentBelt->UpdateSplineComponent();
//						//FHitResult newHit = FHitResult(startPos, endPos);
//						//parentBelt->SetHologramLocationAndRotation(newHit);
//					}
//				}
//			}
//		});
	//-mSnapConnection	0x000002911e891f00 (Name = SnapOnly0)	UFGFactoryConnectionComponent *

	//SUBSCRIBE_METHOD(AFGPoleHologram::SetPoleHeight, [](auto& scope, AFGPoleHologram* self, float height)
	//	{
	//		UE_LOG(LogConstructionPreferences, Display, TEXT("AFGPoleHologram::SetPoleHeight"));
	//		//FDebug::DumpStackTraceToLog(ELogVerbosity::Display);
	//		scope.Cancel();
	//	});

	//void AFGBuildableSubsystem::GetNewNetConstructionID(FNetConstructionID & clientConstructionID) { }
	//SUBSCRIBE_METHOD_AFTER(AFGBuildableSubsystem::GetNewNetConstructionID, [](AFGBuildableSubsystem* self, FNetConstructionID& clientConstructionID)
	//	{
	//		UE_LOG(LogConstructionPreferences, Display, TEXT("AFGBuildableSubsystem::GetNewNetConstructionID"));
	//	});

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
	////virtual void PrimaryFire_Implementation() override;
	////virtual void PrimaryFireRelease_Implementation() override;
	//SUBSCRIBE_METHOD_VIRTUAL(UFGBuildGunStateBuild::PrimaryFire_Implementation, BG, &PrimaryFire)

	//SUBSCRIBE_METHOD_VIRTUAL(UFGBuildGunStateBuild::PrimaryFireRelease_Implementation, BG, [=](auto& scope, UFGBuildGunStateBuild* self)
	//	{
	//		UE_LOG(LogConstructionPreferences, Display, TEXT("UFGBuildGunStateBuild::PrimaryFireRelease_Implementation"));

	//	});
	//SUBSCRIBE_METHOD_AFTER(UFGBuildGunState::PrimaryFireRelease, [](UFGBuildGunState* self)
	//	{
	//		UE_LOG(LogConstructionPreferences, Display, TEXT("UFGBuildGunState::PrimaryFireRelease"));
	//	});
	//SUBSCRIBE_METHOD_AFTER(UFGBuildGunState::PrimaryFire, [](UFGBuildGunState* self)
	//	{
	//		UE_LOG(LogConstructionPreferences, Display, TEXT("UFGBuildGunState::PrimaryFire"));
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
}

IMPLEMENT_GAME_MODULE(FConstructionPreferencesModule, ConstructionPreferences);