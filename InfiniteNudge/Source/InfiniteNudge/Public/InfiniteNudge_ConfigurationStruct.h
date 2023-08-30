#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "InfiniteNudge_ConfigurationStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/InfiniteNudge/InfiniteNudge_Configuration' */
USTRUCT(BlueprintType)
struct FInfiniteNudge_ConfigurationStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    int32 LeftCtrlNudgeAmount;

    UPROPERTY(BlueprintReadWrite)
    int32 LeftAltNudgeAmount;

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FInfiniteNudge_ConfigurationStruct GetActiveConfig() {
        FInfiniteNudge_ConfigurationStruct ConfigStruct{};
        FConfigId ConfigId{"InfiniteNudge", ""};
        UConfigManager* ConfigManager = GEngine->GetEngineSubsystem<UConfigManager>();
        ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FInfiniteNudge_ConfigurationStruct::StaticStruct(), &ConfigStruct});
        return ConfigStruct;
    }
};

