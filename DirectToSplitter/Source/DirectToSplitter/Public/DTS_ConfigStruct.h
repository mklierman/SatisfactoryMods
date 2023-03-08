#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "DTS_ConfigStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/DirectToSplitter/DTS_Config' */
USTRUCT(BlueprintType)
struct FDTS_ConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float SnapOffset;

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FDTS_ConfigStruct GetActiveConfig() {
        FDTS_ConfigStruct ConfigStruct{};
        FConfigId ConfigId{"DirectToSplitter", ""};
        UConfigManager* ConfigManager = GEngine->GetEngineSubsystem<UConfigManager>();
        ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FDTS_ConfigStruct::StaticStruct(), &ConfigStruct});
        return ConfigStruct;
    }
};

