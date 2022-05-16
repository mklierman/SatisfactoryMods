


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

void AHPPR_Subsystem::Mk1Updated()
{
	UE_LOG(HoverPackPoleRange_Log, Display, TEXT("Mk1Updated"));
}

void AHPPR_Subsystem::SetConfigValues()
{
	UE_LOG(HoverPackPoleRange_Log, Display, TEXT("SetConfigValues"));
	auto config = FHPPR_ConfigStruct::GetActiveConfig();
	mMk1Range = config.Mk1;
	mMk2Range = config.Mk2;
	mMk3Range = config.Mk3;
	mRailRange = config.Rails;
	mElseRange = config.EverythingElse;


	////auto config = Cast<UModConfiguration>(ModConfig);


	UConfigManager* ConfigManager = GEngine->GetEngineSubsystem<UConfigManager>();
	FConfigId ConfigId{ "HoverPackPoleRange", "" };
	auto Config = ConfigManager->GetConfigurationById(ConfigId);
	auto ConfigProperty = URuntimeBlueprintFunctionLibrary::GetModConfigurationPropertyByClass(Config);
	auto CPSection = Cast<UConfigPropertySection>(ConfigProperty);
	auto prop = CPSection->SectionProperties["Mk1"];
	FScriptDelegate Delegate;
	Delegate.BindUFunction(this, "Mk1Updated");
	prop->OnPropertyValueChanged.Add(Delegate);
}
