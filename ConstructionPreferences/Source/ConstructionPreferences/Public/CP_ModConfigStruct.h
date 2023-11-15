#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "CP_ModConfigStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/ConstructionPreferences/CP_ModConfig' */
USTRUCT(BlueprintType)
struct FCP_ModConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    int32 PowerPole {};

    UPROPERTY(BlueprintReadWrite)
    int32 WallSocket {};

    UPROPERTY(BlueprintReadWrite)
    int32 ConveyorPole {};

    UPROPERTY(BlueprintReadWrite)
    int32 PipePole {};

    UPROPERTY(BlueprintReadWrite)
    int32 HyperPole {};

    UPROPERTY(BlueprintReadWrite)
    int32 ConveyorLiftHeight {};

    UPROPERTY(BlueprintReadWrite)
    float ConveyorLiftStep {};

    UPROPERTY(BlueprintReadWrite)
    int32 ConveyorLiftHeightMax {};

    UPROPERTY(BlueprintReadWrite)
    bool DisablePowerPoleBuildMode {};

    UPROPERTY(BlueprintReadWrite)
    float ReachDistance {};

    UPROPERTY(BlueprintReadWrite)
    float DismantleDelay {};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FCP_ModConfigStruct GetActiveConfig(UObject* WorldContext) {
        FCP_ModConfigStruct ConfigStruct{};
        FConfigId ConfigId{"ConstructionPreferences", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FCP_ModConfigStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

