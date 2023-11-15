#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "HPPR_ConfigStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/HoverPackPoleRange/HPPR_Config' */
USTRUCT(BlueprintType)
struct FHPPR_ConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    int32 Mk1 {};

    UPROPERTY(BlueprintReadWrite)
    int32 Mk2 {};

    UPROPERTY(BlueprintReadWrite)
    int32 Mk3 {};

    UPROPERTY(BlueprintReadWrite)
    int32 Rails {};

    UPROPERTY(BlueprintReadWrite)
    int32 EverythingElse {};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FHPPR_ConfigStruct GetActiveConfig(UObject* WorldContext) {
        FHPPR_ConfigStruct ConfigStruct{};
        FConfigId ConfigId{"HoverPackPoleRange", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FHPPR_ConfigStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

