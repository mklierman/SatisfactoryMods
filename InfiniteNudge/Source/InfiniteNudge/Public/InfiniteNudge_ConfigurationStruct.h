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
    float LeftCtrlNudgeAmount{};

    UPROPERTY(BlueprintReadWrite)
    int32 LeftAltNudgeAmount{};

    UPROPERTY(BlueprintReadWrite)
    int32 LargeNudgeAmount{};

    UPROPERTY(BlueprintReadWrite)
    float TinyRotateAmount{};

    UPROPERTY(BlueprintReadWrite)
    float SmallRotateAmount{};

    UPROPERTY(BlueprintReadWrite)
    float TinyScaleAmount{};

    UPROPERTY(BlueprintReadWrite)
    float SmallScaleAmount{};

    UPROPERTY(BlueprintReadWrite)
    float LargeScaleAmount{};

    UPROPERTY(BlueprintReadWrite)
    float LargeRotateAmount{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FInfiniteNudge_ConfigurationStruct GetActiveConfig(UObject* WorldContext) {
        FInfiniteNudge_ConfigurationStruct ConfigStruct{};
        FConfigId ConfigId{"InfiniteNudge", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FInfiniteNudge_ConfigurationStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

