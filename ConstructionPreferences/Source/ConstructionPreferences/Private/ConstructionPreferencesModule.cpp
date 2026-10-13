#include "ConstructionPreferencesModule.h"
#include "Patching/NativeHookManager.h"
#include "Hologram/FGConveyorBeltHologram.h"
#include "Hologram/FGBuildableHologram.h"
#include "Subsystem/SubsystemActorManager.h"
#include "Hologram/FGHologram.h"
#include "Equipment/FGBuildGunBuild.h"
#include "Buildables/FGBuildableConveyorBelt.h"
#include "Buildables/FGBuildablePipeline.h"
#include "Buildables/FGBuildablePipeHyper.h"
#include "Buildables/FGBuildableRailroadTrack.h"
#include "FGBuildablePipelineFlowIndicator.h"
#include "FGPipeConnectionComponent.h"
#include "Hologram/FGWireHologram.h"
#include "Hologram/FGRailroadTrackHologram.h"
#include "Holo_WireHologramBuildModes.h"
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
#include "Buildables/FGBuildablePoleStackable.h"
#include "Hologram/FGPipelineHologram.h"
#include "CP_GameInstanceModule.h"
#include "CP_StackableConveyorPoleHologram.h"
#include "CP_StackablePoleHologram.h"
#include "CP_StackablePipelinePoleHologram.h"
#include "FGRecipeManager.h"
#include "FGPlayerController.h"
#include "FGConstructDisqualifier.h"
#include "TimerManager.h"
#include "Hologram/HologramHelpers.h"

namespace
{
	static constexpr float MaxSafeConveyorBeltLength = 5600.0f;
	static constexpr float MaxSafePipelineLength = 5600.0f;
	static constexpr float MaxSafeHypertubeLength = 10000.0f;
	static constexpr float MaxSafeRailroadTrackLength = 10000.0f;

	bool IsControlDown(const AFGHologram* hologram)
	{
		APawn* instigator = IsValid(hologram) ? hologram->GetConstructionInstigator() : nullptr;
		AFGPlayerController* controller = IsValid(instigator) ? Cast<AFGPlayerController>(instigator->GetController()) : nullptr;
		if (!IsValid(controller) && IsValid(hologram) && IsValid(hologram->GetWorld()))
		{
			controller = Cast<AFGPlayerController>(hologram->GetWorld()->GetFirstPlayerController());
		}

		return IsValid(controller) &&
			controller->IsLocalController() &&
			(controller->IsInputKeyDown(EKeys::LeftControl) || controller->IsInputKeyDown(EKeys::RightControl));
	}

	int32 GetActivePoleIndex(const AFGSplineHologram* hologram)
	{
		const ESplineHologramBuildStep buildStep = const_cast<AFGSplineHologram*>(hologram)->GetCurrentBuildStep();
		return buildStep == ESplineHologramBuildStep::SHBS_FindStart ||
			buildStep == ESplineHologramBuildStep::SHBS_AdjustStartingPole ? 0 : 1;
	}

	bool IsStackablePole(const FHitResult& hitResult)
	{
		return IsValid(Cast<AFGBuildablePoleStackable>(hitResult.GetActor()));
	}

	void UpdateStackedPoleDisqualifiers(AFGBuildableHologram* parent, const AFGBuildableHologram* child)
	{
		TArray<TSubclassOf<UFGConstructDisqualifier>> disqualifiers;
		parent->GetConstructDisqualifiers(disqualifiers);
		if (disqualifiers.Remove(UFGCDInvalidFloor::StaticClass()) > 0)
		{
			parent->ResetConstructDisqualifiers();
			for (const TSubclassOf<UFGConstructDisqualifier>& disqualifier : disqualifiers)
			{
				parent->AddConstructDisqualifier(disqualifier);
			}
		}

		TArray<TSubclassOf<UFGConstructDisqualifier>> childDisqualifiers;
		child->GetConstructDisqualifiers(childDisqualifiers);
		for (const TSubclassOf<UFGConstructDisqualifier>& disqualifier : childDisqualifiers)
		{
			if (disqualifier != UFGCDInvalidFloor::StaticClass() && !disqualifiers.Contains(disqualifier))
			{
				parent->AddConstructDisqualifier(disqualifier);
				disqualifiers.Add(disqualifier);
			}
		}
	}

	bool GetSplitData(
		USplineComponent* spline,
		float maxLength,
		float& splitOffset,
		TArray<FSplinePointData>& firstSplineData,
		TArray<FSplinePointData>& secondSplineData)
	{
		if (!IsValid(spline))
		{
			return false;
		}

		const float length = spline->GetSplineLength();
		if (length <= maxLength + 1.0f)
		{
			return false;
		}

		splitOffset = length / FMath::CeilToInt(length / maxLength);
		FSplineUtils::SplitSplineAtDistance(spline, splitOffset, firstSplineData, secondSplineData);
		return firstSplineData.Num() >= 2 && secondSplineData.Num() >= 2;
	}

	template<typename BuildableType, typename SplitterType>
	void SplitLongBuildable(BuildableType* buildable, float maxLength, SplitterType&& splitter)
	{
		TArray<BuildableType*> buildablesToSplit { buildable };
		while (!buildablesToSplit.IsEmpty())
		{
			BuildableType* currentBuildable = buildablesToSplit.Pop(EAllowShrinking::No);
			if (!IsValid(currentBuildable) || currentBuildable->GetLength() <= maxLength + 1.0f)
			{
				continue;
			}

			const float previousLength = currentBuildable->GetLength();
			const int32 segmentCount = FMath::CeilToInt(previousLength / maxLength);
			const float splitOffset = previousLength / segmentCount;
			const TArray<BuildableType*> splitBuildables = splitter(currentBuildable, splitOffset);
			for (BuildableType* splitBuildable : splitBuildables)
			{
				if (IsValid(splitBuildable) &&
					splitBuildable->GetLength() > maxLength + 1.0f &&
					splitBuildable->GetLength() < previousLength - 1.0f)
				{
					buildablesToSplit.Add(splitBuildable);
				}
			}
		}
	}

	void SplitLongConveyorBelt(AFGBuildableConveyorBelt* conveyorBelt)
	{
		if (!IsValid(conveyorBelt))
		{
			return;
		}

		float splitOffset;
		TArray<FSplinePointData> firstSplineData;
		TArray<FSplinePointData> secondSplineData;
		if (!GetSplitData(
			conveyorBelt->GetSplineComponent(),
			MaxSafeConveyorBeltLength,
			splitOffset,
			firstSplineData,
			secondSplineData))
		{
			return;
		}

		const TArray<AFGBuildableConveyorBelt*> splitBelts =
			AFGBuildableConveyorBelt::Split(conveyorBelt, splitOffset, true);
		if (splitBelts.Num() != 2 || !IsValid(splitBelts[0]) || !IsValid(splitBelts[1]))
		{
			return;
		}

		if (!IsValid(AFGBuildableConveyorBelt::Respline(splitBelts[0], firstSplineData)))
		{
			return;
		}

		AFGBuildableConveyorBelt* remainingBelt =
			AFGBuildableConveyorBelt::Respline(splitBelts[1], secondSplineData);
		if (!IsValid(remainingBelt))
		{
			return;
		}

		TWeakObjectPtr<AFGBuildableConveyorBelt> weakRemainingBelt(remainingBelt);
		remainingBelt->GetWorldTimerManager().SetTimerForNextTick(
			[weakRemainingBelt]()
			{
				SplitLongConveyorBelt(weakRemainingBelt.Get());
			});
	}

	template<typename PipeType, typename SplitterType>
	void SplitLongPipe(PipeType* pipe, float maxLength, SplitterType splitter)
	{
		if (!IsValid(pipe))
		{
			return;
		}

		float splitOffset;
		TArray<FSplinePointData> firstSplineData;
		TArray<FSplinePointData> secondSplineData;
		if (!GetSplitData(
			pipe->GetSplineComponent(),
			maxLength,
			splitOffset,
			firstSplineData,
			secondSplineData))
		{
			return;
		}

		const TArray<PipeType*> splitPipes = splitter(pipe, splitOffset);
		if (splitPipes.Num() != 2 || !IsValid(splitPipes[0]) || !IsValid(splitPipes[1]))
		{
			return;
		}

		FConstructionPreferencesModule::SetPipeSpline(splitPipes[0], firstSplineData);
		FConstructionPreferencesModule::SetPipeSpline(splitPipes[1], secondSplineData);

		TWeakObjectPtr<PipeType> weakRemainingPipe(splitPipes[1]);
		splitPipes[1]->GetWorldTimerManager().SetTimerForNextTick(
			[weakRemainingPipe, maxLength, splitter]()
			{
				SplitLongPipe(weakRemainingPipe.Get(), maxLength, splitter);
			});
	}

	void SplitLongSplineBuildable(AActor* buildable)
	{
		if (AFGBuildableConveyorBelt* conveyorBelt = Cast<AFGBuildableConveyorBelt>(buildable))
		{
			SplitLongConveyorBelt(conveyorBelt);
		}
		else if (AFGBuildablePipeline* pipeline = Cast<AFGBuildablePipeline>(buildable))
		{
			SplitLongPipe(pipeline, MaxSafePipelineLength,
				[](AFGBuildablePipeline* pipe, float offset)
				{
					return AFGBuildablePipeline::Split(pipe, offset, true);
				});
		}
		else if (AFGBuildablePipeHyper* hypertube = Cast<AFGBuildablePipeHyper>(buildable))
		{
			SplitLongPipe<AFGBuildablePipeBase>(hypertube, MaxSafeHypertubeLength,
				[](AFGBuildablePipeBase* pipe, float offset)
				{
					TArray<AFGBuildablePipeBase*> splitPipes = AFGBuildablePipeBase::Splice(pipe, offset, 0.0f);
					if (splitPipes.Num() == 2)
					{
						UFGPipeConnectionComponentBase* firstConnection = splitPipes[0]->GetConnection1();
						UFGPipeConnectionComponentBase* secondConnection = splitPipes[1]->GetConnection0();
						if (IsValid(firstConnection) && IsValid(secondConnection) &&
							!firstConnection->IsConnected() && !secondConnection->IsConnected())
						{
							firstConnection->SetConnection(secondConnection);
						}
					}
					return splitPipes;
				});
		}
		else if (AFGBuildableRailroadTrack* railroadTrack = Cast<AFGBuildableRailroadTrack>(buildable))
		{
			SplitLongBuildable(railroadTrack, MaxSafeRailroadTrackLength,
				[](AFGBuildableRailroadTrack* track, float offset)
				{
					return AFGBuildableRailroadTrack::Split(track, offset, true);
				});
		}
	}

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

void FConstructionPreferencesModule::SetPipeSpline(
	AFGBuildablePipeBase* pipe,
	const TArray<FSplinePointData>& splineData)
{
	*pipe->GetMutableSplinePointData() = splineData;
	pipe->GetSplineComponent()->SetSplineData(splineData, ESplineCoordinateSpace::Local);
	pipe->mLength = pipe->GetSplineComponent()->GetSplineLength();
	pipe->Internal_CallRemoveInstances();
	pipe->CallSetupInstances();
	pipe->ApplyCustomizationData_Native(pipe->GetCustomizationData_Native());

	if (AFGBuildablePipeline* pipeline = Cast<AFGBuildablePipeline>(pipe))
	{
		AFGBuildablePipelineFlowIndicator* indicator = pipeline->GetFlowIndicator();
		if (IsValid(indicator))
		{
			pipeline->AddIgnoredActorForIndicator(indicator);
			FTransform indicatorTransform;
			if (!pipeline->FindBestInidicatorPlacement(indicatorTransform))
			{
				indicatorTransform = pipe->GetSplineComponent()->GetTransformAtDistanceAlongSpline(
					pipe->GetSplineComponent()->GetSplineLength() * 0.5f,
					ESplineCoordinateSpace::World);
				indicatorTransform.SetScale3D(indicator->GetActorScale3D());
			}
			AFGBuildablePipelineFlowIndicator* replacement = pipe->GetWorld()->SpawnActorDeferred<AFGBuildablePipelineFlowIndicator>(
				indicator->GetClass(),
				indicatorTransform,
				pipeline,
				nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (IsValid(replacement))
			{
				const FFactoryCustomizationData customizationData = indicator->GetCustomizationData_Native();
				pipeline->mFlowIndicator = nullptr;
				indicator->Destroy();
				replacement->mOwningPipeline = pipeline;
				pipeline->mFlowIndicator = replacement;
				replacement->SetCustomizationData_Native(customizationData);
				replacement->FinishSpawning(indicatorTransform);
				replacement->ForceNetUpdate();
			}
		}
	}

	pipe->ForceNetUpdate();
}

//#pragma optimize("", off)
float FConstructionPreferencesModule::GetUseDistance(const AFGCharacterPlayer* self, float defaultDistance)
{
	USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
	auto reachMult = SessionSettings->GetFloatOptionValue("ConstructionPreferences.UseDistance");
	return reachMult * defaultDistance;
}
//#pragma optimize("", on)

void FConstructionPreferencesModule::StartupModule()
{
#if !WITH_EDITOR
	SUBSCRIBE_METHOD(UFGBuildGunStateBuild::SetActiveRecipe, [this](auto& scope, UFGBuildGunStateBuild* self, TSubclassOf<class UFGRecipe> recipe)
		{
			if (!IsValid(recipe))
			{
				return;
			}
			if (UCP_GameInstanceModule* gameInstanceModule = UCP_GameInstanceModule::Get(GWorld))
			{
				if (IsValid(gameInstanceModule->Build_ConveyorPoleStackable) && IsValid(gameInstanceModule->Holo_ConveyorStackable))
				{
					AFGBuildablePoleStackable* stackable = gameInstanceModule->Build_ConveyorPoleStackable.GetDefaultObject();
					stackable->mHologramClass = ACP_StackablePoleHologram::StaticClass();
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
	SUBSCRIBE_METHOD_VIRTUAL(AFGBuildableHologram::Construct, buildableHologramCDO, [](auto& scope, AFGBuildableHologram* self, TArray<AActor*>& outChildren, FNetConstructionID constructionID)
		{
			if (self->IsA<AFGConveyorBeltHologram>() ||
				self->IsA<AFGPipelineHologram>() ||
				self->IsA<AFGRailroadTrackHologram>())
			{
				AActor* constructedActor = scope(self, outChildren, constructionID);
				if (IsValid(constructedActor) && constructedActor->HasAuthority())
				{
					TWeakObjectPtr<AActor> weakConstructedActor = constructedActor;
					self->GetWorldTimerManager().SetTimerForNextTick(
						[weakConstructedActor]()
						{
							if (AActor* actor = weakConstructedActor.Get())
							{
								SplitLongSplineBuildable(actor);
							}
						});
				}
				scope.Override(constructedActor);
				return;
			}

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

	const TSubclassOf<AFGConveyorBeltHologram> beltHologramClass = LoadClass<AFGConveyorBeltHologram>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/-Shared/Holo_ConveyorBelt.Holo_ConveyorBelt_C"));
	AFGConveyorBeltHologram* beltHologramCDO = IsValid(beltHologramClass)
		? beltHologramClass.GetDefaultObject()
		: GetMutableDefault<AFGConveyorBeltHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorBeltHologram::SpawnChildren, beltHologramCDO, [](auto& scope, AFGConveyorBeltHologram* self, AActor* hologramOwner, FVector spawnLocation, APawn* hologramInstigator)
		{
			USessionSettingsManager* sessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			UCP_GameInstanceModule* gameInstanceModule = UCP_GameInstanceModule::Get(self);
			if (!IsValid(sessionSettings) || !IsValid(gameInstanceModule))
			{
				return;
			}

			if (sessionSettings->GetIntOptionValue("ConstructionPreferences.ConveyorPoleType") == 1)
			{
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
			}

			if (IsValid(gameInstanceModule->Recipe_ConveyorPoleDefault))
			{
				self->mDefaultConveyorPoleRecipe = gameInstanceModule->Recipe_ConveyorPoleDefault;
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorBeltHologram::TrySnapToActor, beltHologramCDO, [](auto& scope, AFGConveyorBeltHologram* self, const FHitResult& hitResult)
		{
			if (!IsControlDown(self) || !IsStackablePole(hitResult))
			{
				return;
			}

			const int32 poleIndex = GetActivePoleIndex(self);
			ACP_StackableConveyorPoleHologram* poleHologram =
				Cast<ACP_StackableConveyorPoleHologram>(self->mChildPoleHologram[poleIndex]);
			if (!IsValid(poleHologram))
			{
				return;
			}

			self->mSnappedConnectionComponents[poleIndex] = nullptr;
			if (poleHologram->TrySnapToActor(hitResult))
			{
				scope.Override(false);
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorBeltHologram::IsValidHitResult, beltHologramCDO, [](auto& scope, const AFGConveyorBeltHologram* self, const FHitResult& hitResult)
		{
			if (!IsControlDown(self) || !IsStackablePole(hitResult))
			{
				return;
			}

			const int32 poleIndex = GetActivePoleIndex(self);
			if (IsValid(Cast<ACP_StackableConveyorPoleHologram>(self->mChildPoleHologram[poleIndex])))
			{
				scope.Override(true);
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorBeltHologram::CanTakeNextBuildStep, beltHologramCDO, [](auto& scope, const AFGConveyorBeltHologram* self)
		{
			const int32 poleIndex = GetActivePoleIndex(self);
			const ACP_StackableConveyorPoleHologram* poleHologram =
				Cast<ACP_StackableConveyorPoleHologram>(self->mChildPoleHologram[poleIndex]);
			if (IsValid(poleHologram) && poleHologram->IsStackedOnPole() && poleHologram->CanConstruct())
			{
				scope.Override(true);
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGConveyorBeltHologram::CheckValidPlacement, beltHologramCDO, [](AFGConveyorBeltHologram* self)
		{
			const int32 poleIndex = GetActivePoleIndex(self);
			const ACP_StackableConveyorPoleHologram* poleHologram =
				Cast<ACP_StackableConveyorPoleHologram>(self->mChildPoleHologram[poleIndex]);
			if (IsValid(poleHologram) && poleHologram->IsStackedOnPole())
			{
				UpdateStackedPoleDisqualifiers(self, poleHologram);
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorBeltHologram::Scroll, beltHologramCDO, [](auto& scope, AFGConveyorBeltHologram* self, int32 delta)
		{
			const int32 poleIndex = GetActivePoleIndex(self);
			ACP_StackableConveyorPoleHologram* poleHologram =
				Cast<ACP_StackableConveyorPoleHologram>(self->mChildPoleHologram[poleIndex]);
			if (IsValid(poleHologram) && poleHologram->IsStackedOnPole())
			{
				poleHologram->RotateStackedPole(delta);
				scope.Cancel();
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGConveyorBeltHologram::BeginPlay, beltHologramCDO, [](AFGConveyorBeltHologram* self)
		{
			USessionSettingsManager* sessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			const float length = sessionSettings->GetFloatOptionValue("ConstructionPreferences.ConveyorBelt.Length");
			if (!FMath::IsNearlyEqual(length, 56.0f, 0.1f))
			{
				self->mMaxSplineLength = length * 100.0f;
			}
		});

	const TSubclassOf<AFGPipelineHologram> pipelineHologramClass = LoadClass<AFGPipelineHologram>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/-Shared/Holo_Pipeline.Holo_Pipeline_C"));
	AFGPipelineHologram* pipelineHologramCDO = IsValid(pipelineHologramClass)
		? pipelineHologramClass.GetDefaultObject()
		: GetMutableDefault<AFGPipelineHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGPipelineHologram::SpawnChildren, pipelineHologramCDO, [](auto& scope, AFGPipelineHologram* self, AActor* hologramOwner, FVector spawnLocation, APawn* hologramInstigator)
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

			if (GetPipelineSupportMode(self) == 1)
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
			}

			if (IsValid(defaultSupportRecipe))
			{
				self->mDefaultPipelineSupportRecipe = defaultSupportRecipe;
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGPipelineHologram::TrySnapToActor, pipelineHologramCDO, [](auto& scope, AFGPipelineHologram* self, const FHitResult& hitResult)
		{
			if (!IsControlDown(self) || !IsStackablePole(hitResult))
			{
				return;
			}

			const int32 poleIndex = GetActivePoleIndex(self);
			ACP_StackablePipelinePoleHologram* poleHologram =
				Cast<ACP_StackablePipelinePoleHologram>(self->mChildPoleHologram[poleIndex]);
			if (!IsValid(poleHologram))
			{
				return;
			}

			self->mSnappedConnectionComponents[poleIndex] = nullptr;
			if (poleHologram->TrySnapToActor(hitResult))
			{
				scope.Override(false);
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGPipelineHologram::IsValidHitResult, pipelineHologramCDO, [](auto& scope, const AFGPipelineHologram* self, const FHitResult& hitResult)
		{
			if (!IsControlDown(self) || !IsStackablePole(hitResult))
			{
				return;
			}

			const int32 poleIndex = GetActivePoleIndex(self);
			const ACP_StackablePipelinePoleHologram* poleHologram =
				Cast<ACP_StackablePipelinePoleHologram>(self->mChildPoleHologram[poleIndex]);
			if (IsValid(poleHologram))
			{
				scope.Override(true);
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGPipelineHologram::CanTakeNextBuildStep, pipelineHologramCDO, [](auto& scope, const AFGPipelineHologram* self)
		{
			const int32 poleIndex = GetActivePoleIndex(self);
			const ACP_StackablePipelinePoleHologram* poleHologram =
				Cast<ACP_StackablePipelinePoleHologram>(self->mChildPoleHologram[poleIndex]);
			if (IsValid(poleHologram) && poleHologram->IsStackedOnPole() && poleHologram->CanConstruct())
			{
				scope.Override(true);
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGPipelineHologram::CheckValidPlacement, pipelineHologramCDO, [](AFGPipelineHologram* self)
		{
			const int32 poleIndex = GetActivePoleIndex(self);
			const ACP_StackablePipelinePoleHologram* poleHologram =
				Cast<ACP_StackablePipelinePoleHologram>(self->mChildPoleHologram[poleIndex]);
			if (IsValid(poleHologram) && poleHologram->IsStackedOnPole())
			{
				UpdateStackedPoleDisqualifiers(self, poleHologram);
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGPipelineHologram::Scroll, pipelineHologramCDO, [](auto& scope, AFGPipelineHologram* self, int32 delta)
		{
			const int32 poleIndex = GetActivePoleIndex(self);
			ACP_StackablePipelinePoleHologram* poleHologram =
				Cast<ACP_StackablePipelinePoleHologram>(self->mChildPoleHologram[poleIndex]);
			if (IsValid(poleHologram) && poleHologram->IsStackedOnPole())
			{
				poleHologram->RotateStackedPole(delta);
				scope.Cancel();
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGPipelineHologram::BeginPlay, pipelineHologramCDO, [](AFGPipelineHologram* self)
		{
			USessionSettingsManager* sessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			const bool bIsHypertube = IsHypertubePipeline(self);
			const float defaultLength = bIsHypertube ? 100.0f : 56.0f;
			const float length = sessionSettings->GetFloatOptionValue(bIsHypertube ? TEXT("ConstructionPreferences.Hypertube.Length") : TEXT("ConstructionPreferences.Pipeline.Length"));
			if (!FMath::IsNearlyEqual(length, defaultLength, 0.1f))
			{
				self->mMaxSplineLength = length * 100.0f;
			}
		});

	AFGRailroadTrackHologram* railroadTrackHologramCDO = GetMutableDefault<AFGRailroadTrackHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGRailroadTrackHologram::BeginPlay, railroadTrackHologramCDO, [](AFGRailroadTrackHologram* self)
		{
			USessionSettingsManager* sessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			if (!IsValid(sessionSettings))
			{
				return;
			}

			const float length = sessionSettings->GetFloatOptionValue("ConstructionPreferences.Railroad.Length");
			if (!FMath::IsNearlyEqual(length, 100.0f, 0.1f))
			{
				self->mMaxLength = length * 100.0f;
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
