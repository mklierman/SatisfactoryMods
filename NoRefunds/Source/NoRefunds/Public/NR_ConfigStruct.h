#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "NR_ConfigStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/NoRefunds/NR_Config' */
USTRUCT(BlueprintType)
struct FNR_ConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    bool Foliage {};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FNR_ConfigStruct GetActiveConfig(UObject* WorldContext) {
        FNR_ConfigStruct ConfigStruct{};
        FConfigId ConfigId{"NoRefunds", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FNR_ConfigStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

