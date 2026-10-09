#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "HPPR_ConfigStruct.generated.h"


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

