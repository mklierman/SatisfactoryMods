#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "NoZFighting_ConfigStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/NoZFighting/NoZFighting_Config' */
USTRUCT(BlueprintType)
struct FNoZFighting_ConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float FoundationMin;

    UPROPERTY(BlueprintReadWrite)
    float FoundationMax;

    UPROPERTY(BlueprintReadWrite)
    float WallMin;

    UPROPERTY(BlueprintReadWrite)
    float WallMax;

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FNoZFighting_ConfigStruct GetActiveConfig() {
        FNoZFighting_ConfigStruct ConfigStruct{};
        FConfigId ConfigId{"NoZFighting", ""};
        UConfigManager* ConfigManager = GEngine->GetEngineSubsystem<UConfigManager>();
        ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FNoZFighting_ConfigStruct::StaticStruct(), &ConfigStruct});
        return ConfigStruct;
    }
};

