#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "OutlineLoot_ConfigurationStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/OutlineLoot/Config/OutlineLoot_Configuration' */
USTRUCT(BlueprintType)
struct FOutlineLoot_ConfigurationStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    int32 OutlineColor{};

    UPROPERTY(BlueprintReadWrite)
    float OutlineSize{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FOutlineLoot_ConfigurationStruct GetActiveConfig(UObject* WorldContext) {
        FOutlineLoot_ConfigurationStruct ConfigStruct{};
        FConfigId ConfigId{"OutlineLoot", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FOutlineLoot_ConfigurationStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

