
#include "CTBP_Component.h"
#include "FGPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "Buildables/FGBuildableGenerator.h"
#include "Buildables/FGBuildableResourceExtractorBase.h"

namespace
{
	void SerializeClipboardSettings(UFGFactoryClipboardSettings* Settings, TArray<uint8>& OutBytes)
	{
		OutBytes.Reset();
		if (!Settings)
		{
			return;
		}

		FMemoryWriter Writer(OutBytes, true);
		FObjectAndNameAsStringProxyArchive Ar(Writer, true);
		Ar.ArNoDelta = true;
		Settings->Serialize(Ar);
	}

	UFGFactoryClipboardSettings* DeserializeClipboardSettings(UObject* Outer, TSubclassOf<UFGFactoryClipboardSettings> SettingsClass, const TArray<uint8>& Bytes)
	{
		if (!Outer || !SettingsClass || Bytes.Num() == 0)
		{
			return nullptr;
		}

		UFGFactoryClipboardSettings* Settings = NewObject<UFGFactoryClipboardSettings>(Outer, SettingsClass);
		TArray<uint8> MutableBytes = Bytes;
		FMemoryReader Reader(MutableBytes, true);
		FObjectAndNameAsStringProxyArchive Ar(Reader, true);
		Ar.ArNoDelta = true;
		Settings->Serialize(Ar);
		return Settings;
	}

	void FillTypedClipboardFields(const UFGFactoryClipboardSettings* Settings, FClipboardData& Data)
	{
		if (const UFGManufacturerClipboardSettings* ManufacturerSettings = Cast<UFGManufacturerClipboardSettings>(Settings))
		{
			Data.Recipe = ManufacturerSettings->mRecipe;
			Data.TargetPotential = ManufacturerSettings->mTargetPotential;
			Data.TargetProductionBoost = ManufacturerSettings->mTargetProductionBoost;
			Data.ReachablePotential = ManufacturerSettings->mReachablePotential;
			Data.ReachableProductionBoost = ManufacturerSettings->mReachableProductionBoost;
			Data.OverclockingShardDescriptor = ManufacturerSettings->mOverclockingShardDescriptor;
			Data.ProductionBoostShardDescriptor = ManufacturerSettings->mProductionBoostShardDescriptor;
			return;
		}

		if (const UFGGeneratorClipboardSettings* GeneratorSettings = Cast<UFGGeneratorClipboardSettings>(Settings))
		{
			Data.TargetPotential = GeneratorSettings->mTargetPotential;
			Data.TargetProductionBoost = GeneratorSettings->mTargetProductionBoost;
			Data.ReachablePotential = GeneratorSettings->mReachablePotential;
			Data.ReachableProductionBoost = GeneratorSettings->mReachableProductionBoost;
			Data.OverclockingShardDescriptor = GeneratorSettings->mOverclockingShardDescriptor;
			Data.ProductionBoostShardDescriptor = GeneratorSettings->mProductionBoostShardDescriptor;
			return;
		}

		if (const UFGResourceExtractorClipboardSettings* ExtractorSettings = Cast<UFGResourceExtractorClipboardSettings>(Settings))
		{
			Data.TargetPotential = ExtractorSettings->mTargetPotential;
			Data.TargetProductionBoost = ExtractorSettings->mTargetProductionBoost;
			Data.ReachablePotential = ExtractorSettings->mReachablePotential;
			Data.ReachableProductionBoost = ExtractorSettings->mReachableProductionBoost;
			Data.OverclockingShardDescriptor = ExtractorSettings->mOverclockingShardDescriptor;
			Data.ProductionBoostShardDescriptor = ExtractorSettings->mProductionBoostShardDescriptor;
		}
	}

	UFGFactoryClipboardSettings* CreateTypedClipboardSettings(UObject* Outer, const FClipboardData& ClipData)
	{
		if (!Outer)
		{
			return nullptr;
		}

		if (ClipData.SettingsClass && ClipData.SettingsClass->IsChildOf(UFGManufacturerClipboardSettings::StaticClass()))
		{
			UFGManufacturerClipboardSettings* Settings = NewObject<UFGManufacturerClipboardSettings>(Outer, ClipData.SettingsClass);
			Settings->mRecipe = ClipData.Recipe;
			Settings->mTargetPotential = ClipData.TargetPotential;
			Settings->mTargetProductionBoost = ClipData.TargetProductionBoost;
			Settings->mReachablePotential = ClipData.ReachablePotential;
			Settings->mReachableProductionBoost = ClipData.ReachableProductionBoost;
			Settings->mOverclockingShardDescriptor = ClipData.OverclockingShardDescriptor;
			Settings->mProductionBoostShardDescriptor = ClipData.ProductionBoostShardDescriptor;
			return Settings;
		}

		if (ClipData.SettingsClass && ClipData.SettingsClass->IsChildOf(UFGGeneratorClipboardSettings::StaticClass()))
		{
			UFGGeneratorClipboardSettings* Settings = NewObject<UFGGeneratorClipboardSettings>(Outer, ClipData.SettingsClass);
			Settings->mTargetPotential = ClipData.TargetPotential;
			Settings->mTargetProductionBoost = ClipData.TargetProductionBoost;
			Settings->mReachablePotential = ClipData.ReachablePotential;
			Settings->mReachableProductionBoost = ClipData.ReachableProductionBoost;
			Settings->mOverclockingShardDescriptor = ClipData.OverclockingShardDescriptor;
			Settings->mProductionBoostShardDescriptor = ClipData.ProductionBoostShardDescriptor;
			return Settings;
		}

		if (ClipData.SettingsClass && ClipData.SettingsClass->IsChildOf(UFGResourceExtractorClipboardSettings::StaticClass()))
		{
			UFGResourceExtractorClipboardSettings* Settings = NewObject<UFGResourceExtractorClipboardSettings>(Outer, ClipData.SettingsClass);
			Settings->mTargetPotential = ClipData.TargetPotential;
			Settings->mTargetProductionBoost = ClipData.TargetProductionBoost;
			Settings->mReachablePotential = ClipData.ReachablePotential;
			Settings->mReachableProductionBoost = ClipData.ReachableProductionBoost;
			Settings->mOverclockingShardDescriptor = ClipData.OverclockingShardDescriptor;
			Settings->mProductionBoostShardDescriptor = ClipData.ProductionBoostShardDescriptor;
			return Settings;
		}

		if (ClipData.Recipe)
		{
			UFGManufacturerClipboardSettings* Settings = NewObject<UFGManufacturerClipboardSettings>(Outer);
			Settings->mRecipe = ClipData.Recipe;
			Settings->mTargetPotential = ClipData.TargetPotential;
			Settings->mTargetProductionBoost = ClipData.TargetProductionBoost;
			Settings->mReachablePotential = ClipData.ReachablePotential;
			Settings->mReachableProductionBoost = ClipData.ReachableProductionBoost;
			Settings->mOverclockingShardDescriptor = ClipData.OverclockingShardDescriptor;
			Settings->mProductionBoostShardDescriptor = ClipData.ProductionBoostShardDescriptor;
			return Settings;
		}

		return nullptr;
	}
}

void UCTBP_Component::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCTBP_Component, ShouldPasteToBP);
	DOREPLIFETIME_CONDITION(UCTBP_Component, PlayerClipboardData, COND_OwnerOnly);
}

UCTBP_Component::UCTBP_Component()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

UCTBP_Component* UCTBP_Component::FindOrAdd(AFGPlayerState* PlayerState)
{
	if (!PlayerState)
	{
		return nullptr;
	}

	if (UCTBP_Component* Existing = PlayerState->FindComponentByClass<UCTBP_Component>())
	{
		return Existing;
	}

	if (!PlayerState->HasAuthority())
	{
		return nullptr;
	}

	UCTBP_Component* Component = NewObject<UCTBP_Component>(PlayerState, TEXT("CTBP_Component"));
	Component->SetIsReplicated(true);
	PlayerState->AddInstanceComponent(Component);
	Component->RegisterComponent();
	return Component;
}

TArray<FClipboardData> UCTBP_Component::MakeClipboardDataFromPlayerState(AFGPlayerState* State)
{
	TArray<FClipboardData> Result;
	if (!State)
	{
		return Result;
	}

	for (const TPair<TSubclassOf<UObject>, TObjectPtr<UFGFactoryClipboardSettings>>& Pair : State->mFactoryClipboard)
	{
		UFGFactoryClipboardSettings* Settings = Pair.Value.Get();
		if (!Settings || !Pair.Key)
		{
			continue;
		}

		FClipboardData Data;
		Data.ObjectClass = Pair.Key;
		Data.SettingsClass = Settings->GetClass();
		FillTypedClipboardFields(Settings, Data);
		SerializeClipboardSettings(Settings, Data.SerializedSettings);
		Result.Add(Data);
	}

	return Result;
}

void UCTBP_Component::ApplyClipboardDataToPlayerState(AFGPlayerState* State, const TArray<FClipboardData>& Data)
{
	if (!State)
	{
		return;
	}

	for (const FClipboardData& ClipData : Data)
	{
		if (!ClipData.ObjectClass)
		{
			continue;
		}

		UFGFactoryClipboardSettings* Settings = DeserializeClipboardSettings(State, ClipData.SettingsClass, ClipData.SerializedSettings);
		if (!Settings)
		{
			Settings = CreateTypedClipboardSettings(State, ClipData);
		}

		if (!Settings)
		{
			continue;
		}

		State->mFactoryClipboard.Add(ClipData.ObjectClass, Settings);
	}
}

void UCTBP_Component::CaptureClipboardFromOwner()
{
	if (AFGPlayerState* State = Cast<AFGPlayerState>(GetOwner()))
	{
		if (State->mFactoryClipboard.Num() > 0)
		{
			PlayerClipboardData = MakeClipboardDataFromPlayerState(State);
		}
	}
}

void UCTBP_Component::ApplyClipboardToOwner()
{
	if (PlayerClipboardData.Num() == 0)
	{
		return;
	}

	ApplyClipboardDataToPlayerState(Cast<AFGPlayerState>(GetOwner()), PlayerClipboardData);
}

void UCTBP_Component::BeginPlay()
{
	Super::BeginPlay();
	ApplyClipboardToOwner();
}

void UCTBP_Component::PreSaveGame_Implementation(int32 SaveVersion, int32 GameVersion)
{
	CaptureClipboardFromOwner();
}

void UCTBP_Component::PostLoadGame_Implementation(int32 SaveVersion, int32 GameVersion)
{
	ApplyClipboardToOwner();
}

void UCTBP_Component::OnRep_PlayerClipboardData()
{
	ApplyClipboardToOwner();
}

void UCTBP_Component::ServerSetPlayerClipboardData_Implementation(const TArray<FClipboardData>& data)
{
	PlayerClipboardData = data;
}

void UCTBP_Component::ServerSetShouldPasteToBP_Implementation(bool bNewValue)
{
	ShouldPasteToBP = bNewValue;
}
