#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "IndicatorColors_ConfigurationStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/IndicatorColors/IndicatorColors_Configuration' */
USTRUCT(BlueprintType)
struct FIndicatorColors_ConfigurationStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    int32 ErrorColor{};

    UPROPERTY(BlueprintReadWrite)
    int32 ActiveColor{};

    UPROPERTY(BlueprintReadWrite)
    int32 StandbyColor{};

    UPROPERTY(BlueprintReadWrite)
    int32 OverclockColor{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FIndicatorColors_ConfigurationStruct GetActiveConfig(UObject* WorldContext) {
        FIndicatorColors_ConfigurationStruct ConfigStruct{};
        FConfigId ConfigId{"IndicatorColors", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FIndicatorColors_ConfigurationStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

