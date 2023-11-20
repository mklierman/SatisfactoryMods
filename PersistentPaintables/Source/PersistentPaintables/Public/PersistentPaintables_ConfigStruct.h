#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "PersistentPaintables_ConfigStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/PersistentPaintables/PersistentPaintables_Config' */
USTRUCT(BlueprintType)
struct FPersistentPaintables_ConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    bool PaintPipes{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FPersistentPaintables_ConfigStruct GetActiveConfig(UObject* WorldContext) {
        FPersistentPaintables_ConfigStruct ConfigStruct{};
        FConfigId ConfigId{"PersistentPaintables", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FPersistentPaintables_ConfigStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

