// Copyright Epic Games, Inc. All Rights Reserved.

#include "CopyToBP.h"
#include "Logging/StructuredLog.h"
#include "Patching/NativeHookManager.h"
#include "Hologram/FGBlueprintHologram.h"
#include "FGFactoryClipboard.h"
#include "FGCharacterPlayer.h"
#include "FGPlayerState.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "CTBP_Component.h"

#define LOCTEXT_NAMESPACE "FCopyToBPModule"
DEFINE_LOG_CATEGORY(CTBP_Log);

namespace
{
	TArray<FClipboardData> MakeClipboardDataFromPlayerState(AFGPlayerState* State)
	{
		TArray<FClipboardData> Result;
		if (!State)
		{
			return Result;
		}

		for (const TPair<TSubclassOf<UObject>, TObjectPtr<UFGFactoryClipboardSettings>>& Pair : State->mFactoryClipboard)
		{
			const UFGManufacturerClipboardSettings* ManufacturerSettings = Cast<UFGManufacturerClipboardSettings>(Pair.Value.Get());
			if (!ManufacturerSettings || !Pair.Key)
			{
				continue;
			}

			FClipboardData Data;
			Data.ObjectClass = Pair.Key;
			Data.Recipe = ManufacturerSettings->mRecipe;
			Data.TargetPotential = ManufacturerSettings->mTargetPotential;
			Data.TargetProductionBoost = ManufacturerSettings->mTargetProductionBoost;
			Data.ReachablePotential = ManufacturerSettings->mReachablePotential;
			Data.ReachableProductionBoost = ManufacturerSettings->mReachableProductionBoost;
			Data.OverclockingShardDescriptor = ManufacturerSettings->mOverclockingShardDescriptor;
			Data.ProductionBoostShardDescriptor = ManufacturerSettings->mProductionBoostShardDescriptor;
			Result.Add(Data);
		}

		return Result;
	}

	void ApplyClipboardDataToPlayerState(AFGPlayerState* State, const TArray<FClipboardData>& Data)
	{
		if (!State)
		{
			return;
		}

		TMap<TSubclassOf<UObject>, TObjectPtr<UFGFactoryClipboardSettings>> Clipboard;
		for (const FClipboardData& ClipData : Data)
		{
			if (!ClipData.ObjectClass)
			{
				continue;
			}

			UFGManufacturerClipboardSettings* Settings = NewObject<UFGManufacturerClipboardSettings>(State);
			Settings->mRecipe = ClipData.Recipe;
			Settings->mTargetPotential = ClipData.TargetPotential;
			Settings->mTargetProductionBoost = ClipData.TargetProductionBoost;
			Settings->mReachablePotential = ClipData.ReachablePotential;
			Settings->mReachableProductionBoost = ClipData.ReachableProductionBoost;
			Settings->mOverclockingShardDescriptor = ClipData.OverclockingShardDescriptor;
			Settings->mProductionBoostShardDescriptor = ClipData.ProductionBoostShardDescriptor;
			Clipboard.Add(ClipData.ObjectClass, Settings);
		}

		State->mFactoryClipboard = Clipboard;
	}

	AFGPlayerState* GetPlayerStateFromInstigator(APawn* InstigatorPawn)
	{
		if (!InstigatorPawn)
		{
			return nullptr;
		}

		if (AFGCharacterPlayer* Character = Cast<AFGCharacterPlayer>(InstigatorPawn))
		{
			if (AFGPlayerState* State = Character->GetControllingPlayerState())
			{
				return State;
			}
		}

		if (AController* Controller = InstigatorPawn->GetController())
		{
			return Controller->GetPlayerState<AFGPlayerState>();
		}

		return nullptr;
	}
}

void FCopyToBPModule::StartupModule()
{
#if !WITH_EDITOR
	AFGPlayerState* PlayerStateCDO = GetMutableDefault<AFGPlayerState>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGPlayerState::BeginPlay, PlayerStateCDO, [](AActor* self)
		{
			UCTBP_Component::FindOrAdd(Cast<AFGPlayerState>(self));
		});

	AFGBlueprintHologram* bph = GetMutableDefault<AFGBlueprintHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBlueprintHologram::Construct, bph, [this](AActor* result, AFGBlueprintHologram* self, TArray< AActor* >& out_children, FNetConstructionID NetConstructionID)
		{
			if (!self || !self->HasAuthority())
			{
				return;
			}

			//UE_LOGFMT(CTBP_Log, Display, "AFGBlueprintHologram::Construct");

			AFGPlayerState* State = GetPlayerStateFromInstigator(self->GetConstructionInstigator());
			if (!State)
			{
				//UE_LOGFMT(CTBP_Log, Warning, "AFGBlueprintHologram::Construct missing player state");
				return;
			}

			UCTBP_Component* ccomp = State->FindComponentByClass<UCTBP_Component>();
			if (!ccomp)
			{
				//UE_LOGFMT(CTBP_Log, Warning, "AFGBlueprintHologram::Construct missing UCTBP_Component");
				return;
			}

			//UE_LOGFMT(CTBP_Log, Display, "AFGBlueprintHologram::Construct Found UCTBP_Component {0}", ccomp->GetName());

			if (!ccomp->ShouldPasteToBP)
			{
				return;
			}

			ApplyClipboardDataToPlayerState(State, ccomp->PlayerClipboardData);

			for (AActor* Child : out_children)
			{
				if (AFGBuildableManufacturer* Buildable = Cast<AFGBuildableManufacturer>(Child))
				{
					State->PasteFactoryClipboard(Child);
					//UE_LOGFMT(CTBP_Log, Display, "Pasted to {0}", Buildable->GetName());
				}
			}
		});

	SUBSCRIBE_METHOD_AFTER(AFGPlayerState::CopyFactoryClipboard, [this](AFGPlayerState* self, UObject* object)
		{
			if (!self)
			{
				return;
			}

			//UE_LOGFMT(CTBP_Log, Display, "CopyFactoryClipboard");

			UCTBP_Component* ccomp = UCTBP_Component::FindOrAdd(self);
			if (!ccomp)
			{
				//UE_LOGFMT(CTBP_Log, Warning, "CopyFactoryClipboard missing UCTBP_Component");
				return;
			}

			//UE_LOGFMT(CTBP_Log, Display, "CopyFactoryClipboard Found UCTBP_Component {0}", ccomp->GetName());

			const TArray<FClipboardData> Data = MakeClipboardDataFromPlayerState(self);
			ccomp->PlayerClipboardData = Data;
			ccomp->ServerSetPlayerClipboardData(Data);
			ccomp->ServerSetShouldPasteToBP(ccomp->ShouldPasteToBP);
		});
#endif
}

void FCopyToBPModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCopyToBPModule, CopyToBP)
