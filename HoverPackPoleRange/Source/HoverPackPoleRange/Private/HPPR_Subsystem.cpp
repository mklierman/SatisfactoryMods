#include "HPPR_Subsystem.h"
#include "HoverPackPoleRangeModule.h"
#include "FGPowerConnectionComponent.h"
#include "Equipment/FGHoverPack.h"
#include "Patching/NativeHookManager.h"
#include "FGCircuitConnectionComponent.h"
#include "FGPowerInfoComponent.h"
#include "Buildables/FGBuildablePowerPole.h"
#include "Equipment/FGEquipment.h"
#include "FGCharacterPlayer.h"
#include "HPPR_ConfigStruct.h"
#include "Util/RuntimeBlueprintFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Configuration/ConfigProperty.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "Configuration/Properties/ConfigPropertySection.h"
#include "FGGameMode.h"

void AHPPR_Subsystem::Loggit(FString myString)
{
	if (debugLogging)
	{
		UE_LOG(HoverPackPoleRange_Log, Display, TEXT("%s"), *myString);
	}
}

void AHPPR_Subsystem::Mk1Updated()
{
	auto ModConfig = FHPPR_ConfigStruct::GetActiveConfig();
	mMk1Range = ModConfig.Mk1;
	//Loggit("mk1");
}

void AHPPR_Subsystem::Mk2Updated()
{
	auto ModConfig = FHPPR_ConfigStruct::GetActiveConfig();
	mMk2Range = ModConfig.Mk2;
	//Loggit("mk2");
}

void AHPPR_Subsystem::Mk3Updated()
{
	auto ModConfig = FHPPR_ConfigStruct::GetActiveConfig();
	mMk3Range = ModConfig.Mk3;
	//Loggit("mk3");
}

void AHPPR_Subsystem::RailsUpdated()
{
	auto ModConfig = FHPPR_ConfigStruct::GetActiveConfig();
	mRailRange = ModConfig.Rails;
	//Loggit("rails");
}

void AHPPR_Subsystem::EverythingElseUpdated()
{
	auto ModConfig = FHPPR_ConfigStruct::GetActiveConfig();
	mElseRange = ModConfig.EverythingElse;
	//Loggit("else");
}

void AHPPR_Subsystem::SetConfigValues()
{
	debugLogging = true;
	//Loggit("SetConfigValues");

	auto ModConfig = FHPPR_ConfigStruct::GetActiveConfig();
	mMk1Range = ModConfig.Mk1;
	mMk2Range = ModConfig.Mk2;
	mMk3Range = ModConfig.Mk3;
	mRailRange = ModConfig.Rails;
	mElseRange = ModConfig.EverythingElse;

	UConfigManager* ConfigManager = GEngine->GetEngineSubsystem<UConfigManager>();
	FConfigId ConfigId{ "HoverPackPoleRange", "" };
	auto Config = ConfigManager->GetConfigurationById(ConfigId);
	auto ConfigProperty = URuntimeBlueprintFunctionLibrary::GetModConfigurationPropertyByClass(Config);
	auto CPSection = Cast<UConfigPropertySection>(ConfigProperty);

	auto mk1Prop = CPSection->SectionProperties["Mk1"];
	auto mk2Prop = CPSection->SectionProperties["Mk2"];
	auto mk3Prop = CPSection->SectionProperties["Mk3"];
	auto railProp = CPSection->SectionProperties["Rails"];
	auto elseProp = CPSection->SectionProperties["EverythingElse"];

	FScriptDelegate mk1Delegate;
	mk1Delegate.BindUFunction(this, "Mk1Updated");
	mk1Prop->OnPropertyValueChanged.AddUnique(mk1Delegate);

	FScriptDelegate mk2Delegate;
	mk2Delegate.BindUFunction(this, "Mk2Updated");
	mk2Prop->OnPropertyValueChanged.AddUnique(mk2Delegate);

	FScriptDelegate mk3Delegate;
	mk3Delegate.BindUFunction(this, "Mk3Updated");
	mk3Prop->OnPropertyValueChanged.AddUnique(mk3Delegate);

	FScriptDelegate railsDelegate;
	railsDelegate.BindUFunction(this, "RailsUpdated");
	railProp->OnPropertyValueChanged.AddUnique(railsDelegate);

	FScriptDelegate elseDelegate;
	elseDelegate.BindUFunction(this, "EverythingElseUpdated");
	elseProp->OnPropertyValueChanged.AddUnique(elseDelegate);
}
