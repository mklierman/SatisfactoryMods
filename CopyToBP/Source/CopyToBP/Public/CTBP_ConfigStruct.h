#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "CTBP_ConfigStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/CopyToBP/Configs/CTBP_Config' */
USTRUCT(BlueprintType)
struct FCTBP_ConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    bool ShouldPaste{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FCTBP_ConfigStruct GetActiveConfig(UObject* WorldContext) {
        FCTBP_ConfigStruct ConfigStruct{};
        FConfigId ConfigId{"CopyToBP", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FCTBP_ConfigStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

