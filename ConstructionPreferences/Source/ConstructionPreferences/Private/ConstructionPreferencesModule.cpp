#include "ConstructionPreferencesModule.h"
#include "Patching/NativeHookManager.h"
#include "Hologram/FGConveyorBeltHologram.h"
#include "Hologram/FGBuildableHologram.h"
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
#include "CP_Subsystem.h"
#include "FGBuildableSubsystem.h"
#include "Equipment/FGBuildGun.h"
#include "Equipment/FGParachute.h"
#include "Hologram/FGBlueprintHologram.h"
#include <SessionSettings/SessionSettingsManager.h>
#include <Logging/StructuredLog.h>
#include "Buildables/FGBuildablePoleStackable.h"
#include "Hologram/FGStackablePoleHologram.h"
#include "Hologram/FGPipelineHologram.h"
#include "Hologram/FGPipelinePoleHologram.h"
#include "CP_GameInstanceModule.h"
#include "CP_StackableConveyorPoleHologram.h"
#include "CP_StackablePipelinePoleHologram.h"
#include "FGRecipeManager.h"
#include "TimerManager.h"

namespace
{
	void RegisterInternalRecipe(AFGRecipeManager* recipeManager, TSubclassOf<UFGRecipe> recipe)
	{
		if (!IsValid(recipeManager) || !IsValid(recipe))
		{
			return;
		}

		if (recipeManager->IsRecipeAvailable(recipe))
		{
			TArray<TSubclassOf<UFGRecipe>> recipesToRemove { recipe };
			recipeManager->RemoveAvailableRecipes(recipesToRemove);
		}
		recipeManager->AddAvailableRecipe(recipe);
	}

	bool IsHypertubePipeline(const AFGPipelineHologram* pipelineHologram)
	{
		const TSubclassOf<AActor> buildClass = pipelineHologram->GetActorClass();
		return IsValid(buildClass) && buildClass->GetName().Contains(TEXT("Hyper"), ESearchCase::IgnoreCase);
	}

	int32 GetPipelineSupportMode(const AFGPipelineHologram* pipelineHologram)
	{
		if (USessionSettingsManager* sessionSettings =
			pipelineHologram->GetWorld()->GetSubsystem<USessionSettingsManager>())
		{
			return sessionSettings->GetIntOptionValue(
				IsHypertubePipeline(pipelineHologram)
					? TEXT("ConstructionPreferences.HyperTubePoleType")
					: TEXT("ConstructionPreferences.PipePoleType"));
		}
		return 0;
	}
}

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
	SUBSCRIBE_METHOD(UFGBuildGunStateBuild::SetActiveRecipe, [this](auto& scope, UFGBuildGunStateBuild* self, TSubclassOf< class UFGRecipe > recipe)
		{
			if (!IsValid(recipe)) {
				return;
			}
			if (UCP_GameInstanceModule* gameInstanceModule = UCP_GameInstanceModule::Get(GWorld))
			{
				if (IsValid(gameInstanceModule->Build_ConveyorPoleStackable) && IsValid(gameInstanceModule->Holo_ConveyorStackable))
				{
					AFGBuildablePoleStackable* stackable = gameInstanceModule->Build_ConveyorPoleStackable.GetDefaultObject();
					stackable->mHologramClass = gameInstanceModule->Holo_ConveyorStackable;
				}
			}
		});

	AFGRecipeManager* recipeManagerCDO = GetMutableDefault<AFGRecipeManager>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGRecipeManager::BeginPlay, recipeManagerCDO, [](AFGRecipeManager* self)
		{
			if (self->GetWorld()->GetNetMode() == NM_Client)
			{
				return;
			}

			if (UCP_GameInstanceModule* gameInstanceModule = UCP_GameInstanceModule::Get(self))
			{
				RegisterInternalRecipe(self, gameInstanceModule->Recipe_ConveyorPoleBeltStackable);
			}
		});

	AFGBuildableHologram* buildableHologramCDO = GetMutableDefault<AFGBuildableHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGBuildableHologram::Construct, buildableHologramCDO,
		[](auto& scope, AFGBuildableHologram* self, TArray<AActor*>& outChildren, FNetConstructionID constructionID)
		{
			bool shouldDestroySupport = false;
			if (IsValid(Cast<AFGConveyorBeltHologram>(self->GetParentHologram())))
			{
				if (USessionSettingsManager* sessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>())
				{
					shouldDestroySupport =
						sessionSettings->GetIntOptionValue("ConstructionPreferences.ConveyorPoleType") == 2;
				}
			}
			else if (const AFGPipelineHologram* parentPipeline =
				Cast<AFGPipelineHologram>(self->GetParentHologram()))
			{
				shouldDestroySupport = GetPipelineSupportMode(parentPipeline) == 2;
			}

			if (!shouldDestroySupport)
			{
				return;
			}

			AActor* temporarySupport = scope(self, outChildren, constructionID);
			if (IsValid(temporarySupport))
			{
				TWeakObjectPtr<AActor> weakTemporarySupport = temporarySupport;
				self->GetWorld()->GetTimerManager().SetTimerForNextTick(
					FTimerDelegate::CreateLambda([weakTemporarySupport]()
						{
							if (weakTemporarySupport.IsValid())
							{
								weakTemporarySupport->Destroy();
							}
						}));
			}
			scope.Override(temporarySupport);
		});

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

	const TSubclassOf<AFGConveyorBeltHologram> beltHologramClass = LoadClass<AFGConveyorBeltHologram>(
		nullptr,
		TEXT("/Game/FactoryGame/Buildable/Factory/-Shared/Holo_ConveyorBelt.Holo_ConveyorBelt_C"));
	AFGConveyorBeltHologram* beltHologramCDO = IsValid(beltHologramClass)
		? beltHologramClass.GetDefaultObject()
		: GetMutableDefault<AFGConveyorBeltHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorBeltHologram::SpawnChildren, beltHologramCDO,
		[](auto& scope, AFGConveyorBeltHologram* self, AActor* hologramOwner, FVector spawnLocation, APawn* hologramInstigator)
		{
			USessionSettingsManager* sessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			UCP_GameInstanceModule* gameInstanceModule = UCP_GameInstanceModule::Get(self);
			if (!IsValid(sessionSettings) || !IsValid(gameInstanceModule))
			{
				return;
			}

			switch (sessionSettings->GetIntOptionValue("ConstructionPreferences.ConveyorPoleType"))
			{
			case 1:
				if (AFGRecipeManager* recipeManager = AFGRecipeManager::Get(self);
					IsValid(recipeManager) &&
					recipeManager->IsRecipeAvailable(gameInstanceModule->Recipe_ConveyorPoleStackable) &&
					IsValid(gameInstanceModule->Build_ConveyorPoleStackable))
				{
					AFGBuildablePoleStackable* stackableCDO = gameInstanceModule->Build_ConveyorPoleStackable.GetDefaultObject();
					const TSubclassOf<AFGHologram> previousHologramClass = stackableCDO->mHologramClass;
					stackableCDO->mHologramClass = ACP_StackableConveyorPoleHologram::StaticClass();
					self->mDefaultConveyorPoleRecipe = gameInstanceModule->Recipe_ConveyorPoleStackable;

					scope(self, hologramOwner, spawnLocation, hologramInstigator);
					stackableCDO->mHologramClass = previousHologramClass;
					return;
				}
				break;
			case 2:
				if (IsValid(gameInstanceModule->Recipe_ConveyorPoleDefault))
				{
					self->mDefaultConveyorPoleRecipe = gameInstanceModule->Recipe_ConveyorPoleDefault;
				}
				break;
			default:
				if (IsValid(gameInstanceModule->Recipe_ConveyorPoleDefault))
				{
					self->mDefaultConveyorPoleRecipe = gameInstanceModule->Recipe_ConveyorPoleDefault;
				}
				break;
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGConveyorBeltHologram::BeginPlay, beltHologramCDO, [](AFGConveyorBeltHologram* self)
		{
			USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			auto length = SessionSettings->GetFloatOptionValue("ConstructionPreferences.ConveyorBelt.Length");
			if (!FMath::IsNearlyEqual(length, 56, 0.1)) // Check if default
			{
				self->mMaxSplineLength = length * 100;
			}
		});

	const TSubclassOf<AFGPipelineHologram> pipelineHologramClass = LoadClass<AFGPipelineHologram>(
		nullptr,
		TEXT("/Game/FactoryGame/Buildable/Factory/-Shared/Holo_Pipeline.Holo_Pipeline_C"));
	AFGPipelineHologram* pipelineHologramCDO = IsValid(pipelineHologramClass)
		? pipelineHologramClass.GetDefaultObject()
		: GetMutableDefault<AFGPipelineHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGPipelineHologram::SpawnChildren, pipelineHologramCDO,
		[](auto& scope, AFGPipelineHologram* self, AActor* hologramOwner, FVector spawnLocation, APawn* hologramInstigator)
		{
			UCP_GameInstanceModule* gameInstanceModule = UCP_GameInstanceModule::Get(self);
			if (!IsValid(gameInstanceModule))
			{
				return;
			}

			const bool bIsHypertube = IsHypertubePipeline(self);
			const TSubclassOf<UFGRecipe> defaultSupportRecipe = bIsHypertube
				? gameInstanceModule->Recipe_HypertubeSupportDefault
				: gameInstanceModule->Recipe_PipelineSupportDefault;

			switch (GetPipelineSupportMode(self))
			{
			case 1:
			{
				const TSubclassOf<UFGRecipe> stackableRecipe = bIsHypertube
					? gameInstanceModule->Recipe_HypertubeSupportStackable
					: gameInstanceModule->Recipe_PipelineSupportStackable;
				const TSubclassOf<AFGBuildablePoleStackable> stackableBuildable = bIsHypertube
					? gameInstanceModule->Build_HypertubeSupportStackable
					: gameInstanceModule->Build_PipelineSupportStackable;

				if (AFGRecipeManager* recipeManager = AFGRecipeManager::Get(self);
					IsValid(recipeManager) &&
					IsValid(stackableRecipe) &&
					IsValid(stackableBuildable) &&
					recipeManager->IsRecipeAvailable(stackableRecipe))
				{
					AFGBuildablePoleStackable* stackableCDO = stackableBuildable.GetDefaultObject();
					const TSubclassOf<AFGHologram> previousHologramClass = stackableCDO->mHologramClass;
					stackableCDO->mHologramClass = ACP_StackablePipelinePoleHologram::StaticClass();
					self->mDefaultPipelineSupportRecipe = stackableRecipe;

					scope(self, hologramOwner, spawnLocation, hologramInstigator);
					stackableCDO->mHologramClass = previousHologramClass;
					return;
				}
				break;
			}
			case 2:
			default:
				break;
			}

			if (IsValid(defaultSupportRecipe))
			{
				self->mDefaultPipelineSupportRecipe = defaultSupportRecipe;
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGPipelineHologram::BeginPlay, pipelineHologramCDO, [](AFGPipelineHologram* self)
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

	//AFGRailroadTrackHologram* rrhg = GetMutableDefault<AFGRailroadTrackHologram>();
	//SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGRailroadTrackHologram::BeginPlay, rrhg, [](AFGRailroadTrackHologram* self)
	//	{
	//		USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
	//		auto length = SessionSettings->GetFloatOptionValue("ConstructionPreferences.Railroad.Length");
	//		if (!FMath::IsNearlyEqual(length, 100, 0.1)) // Check if default
	//		{
	//			self->mMaxLength = length * 100;
	//			//UE_LOGFMT(LogConstructionPreferences, Display, "Railroad Max Length: {0}", self->mMaxLength);
	//		}
	//	});

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
