#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "SD_ConfigStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/StackableDistance/SD_Config' */
USTRUCT(BlueprintType)
struct FSD_ConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float SafetyBuffer{};

    UPROPERTY(BlueprintReadWrite)
    float CurveCompensation{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FSD_ConfigStruct GetActiveConfig(UObject* WorldContext) {
        FSD_ConfigStruct ConfigStruct{};
        FConfigId ConfigId{"StackableDistance", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FSD_ConfigStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

