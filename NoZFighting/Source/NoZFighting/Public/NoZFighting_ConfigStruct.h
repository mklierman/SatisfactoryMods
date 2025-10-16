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
    float FoundationMin {};

    UPROPERTY(BlueprintReadWrite)
    float FoundationMax {};

    UPROPERTY(BlueprintReadWrite)
    float WallMin {};

    UPROPERTY(BlueprintReadWrite)
    float WallMax {};

    UPROPERTY(BlueprintReadWrite)
    float BeamMin {};

    UPROPERTY(BlueprintReadWrite)
    float BeamMax {};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FNoZFighting_ConfigStruct GetActiveConfig(UObject* WorldContext) {
        FNoZFighting_ConfigStruct ConfigStruct{};
        FConfigId ConfigId{"NoZFighting", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FNoZFighting_ConfigStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

