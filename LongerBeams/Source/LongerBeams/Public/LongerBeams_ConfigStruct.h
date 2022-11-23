#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "LongerBeams_ConfigStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/LongerBeams/LongerBeams_Config' */
USTRUCT(BlueprintType)
struct FLongerBeams_ConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float BeamLength;

    UPROPERTY(BlueprintReadWrite)
    float ScrollAmount;

    UPROPERTY(BlueprintReadWrite)
    int32 RotationAmount;

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FLongerBeams_ConfigStruct GetActiveConfig() {
        FLongerBeams_ConfigStruct ConfigStruct{};
        FConfigId ConfigId{"LongerBeams", ""};
        UConfigManager* ConfigManager = GEngine->GetEngineSubsystem<UConfigManager>();
        ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FLongerBeams_ConfigStruct::StaticStruct(), &ConfigStruct});
        return ConfigStruct;
    }
};

