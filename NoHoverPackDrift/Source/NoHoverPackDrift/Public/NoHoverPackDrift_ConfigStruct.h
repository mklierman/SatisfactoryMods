#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "NoHoverPackDrift_ConfigStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/NoHoverPackDrift/NoHoverPackDrift_Config' */
USTRUCT(BlueprintType)
struct FNoHoverPackDrift_ConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float MinSpeed;

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FNoHoverPackDrift_ConfigStruct GetActiveConfig() {
        FNoHoverPackDrift_ConfigStruct ConfigStruct{};
        FConfigId ConfigId{"NoHoverPackDrift", ""};
        UConfigManager* ConfigManager = GEngine->GetEngineSubsystem<UConfigManager>();
        ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FNoHoverPackDrift_ConfigStruct::StaticStruct(), &ConfigStruct});
        return ConfigStruct;
    }
};

