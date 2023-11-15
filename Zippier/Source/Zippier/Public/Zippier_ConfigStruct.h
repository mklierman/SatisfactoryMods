#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "Zippier_ConfigStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/Zippier/Zippier_Config' */
USTRUCT(BlueprintType)
struct FZippier_ConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float ZiplineBaseSpeed {};

    UPROPERTY(BlueprintReadWrite)
    float ZiplineSprintSpeed {};

    UPROPERTY(BlueprintReadWrite)
    float ZiplineUpSpeedMult {};

    UPROPERTY(BlueprintReadWrite)
    float ZiplineDownSpeedMult {};

    UPROPERTY(BlueprintReadWrite)
    float ZiplineCheckAngle {};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FZippier_ConfigStruct GetActiveConfig(UObject* WorldContext) {
        FZippier_ConfigStruct ConfigStruct{};
        FConfigId ConfigId{"Zippier", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FZippier_ConfigStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

