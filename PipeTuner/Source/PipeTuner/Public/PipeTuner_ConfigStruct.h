#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "PipeTuner_ConfigStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/PipeTuner/PipeTuner_Config' */
USTRUCT(BlueprintType)
struct FPipeTuner_ConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float Mk1PipeVolumeMultiplier {};

    UPROPERTY(BlueprintReadWrite)
    float Mk2PipeVolumeMultiplier {};

    UPROPERTY(BlueprintReadWrite)
    float OverfillPercent {};

    UPROPERTY(BlueprintReadWrite)
    float OverfillPressurePercent {};

    UPROPERTY(BlueprintReadWrite)
    float PressureLossPercent {};

    UPROPERTY(BlueprintReadWrite)
    int32 PumpTraversalDistance {};

    UPROPERTY(BlueprintReadWrite)
    int32 PumpJunctionRecursions {};

    UPROPERTY(BlueprintReadWrite)
    int32 PumpOffsetEstimationCount {};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FPipeTuner_ConfigStruct GetActiveConfig(UObject* WorldContext) {
        FPipeTuner_ConfigStruct ConfigStruct{};
        FConfigId ConfigId{"PipeTuner", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FPipeTuner_ConfigStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

