#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "InfiniteZoop_ConfigurationStruct.generated.h"

struct FInfiniteZoop_ConfigurationStruct_BuildEffects;

USTRUCT(BlueprintType)
struct FInfiniteZoop_ConfigurationStruct_BuildEffects {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    bool SkipEffects{};

    UPROPERTY(BlueprintReadWrite)
    int32 SkipAfterAmount{};
};

/* Struct generated from Mod Configuration Asset '/InfiniteZoop/InfiniteZoop_Configuration' */
USTRUCT(BlueprintType)
struct FInfiniteZoop_ConfigurationStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    bool CheatMode{};

    UPROPERTY(BlueprintReadWrite)
    bool Ladders{};

    UPROPERTY(BlueprintReadWrite)
    bool Corners{};

    UPROPERTY(BlueprintReadWrite)
    FInfiniteZoop_ConfigurationStruct_BuildEffects BuildEffects{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FInfiniteZoop_ConfigurationStruct GetActiveConfig(UObject* WorldContext) {
        FInfiniteZoop_ConfigurationStruct ConfigStruct{};
        FConfigId ConfigId{"InfiniteZoop", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FInfiniteZoop_ConfigurationStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

