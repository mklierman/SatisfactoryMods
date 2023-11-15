#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "UnifiedGrid_ConfigStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/UnifiedGrid/UnifiedGrid_Config' */
USTRUCT(BlueprintType)
struct FUnifiedGrid_ConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    int32 AlignmentHeight {};

    UPROPERTY(BlueprintReadWrite)
    int32 AdditionalHeight {};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FUnifiedGrid_ConfigStruct GetActiveConfig(UObject* WorldContext) {
        FUnifiedGrid_ConfigStruct ConfigStruct{};
        FConfigId ConfigId{"UnifiedGrid", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FUnifiedGrid_ConfigStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

