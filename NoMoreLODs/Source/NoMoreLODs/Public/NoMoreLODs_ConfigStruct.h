#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "NoMoreLODs_ConfigStruct.generated.h"

USTRUCT(BlueprintType)
struct FNoMoreLODs_ConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    bool LoadEntireMap{};

    static FNoMoreLODs_ConfigStruct GetActiveConfig(UObject* WorldContext) {
        FNoMoreLODs_ConfigStruct ConfigStruct{};
        FConfigId ConfigId{"NoMoreLODs", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FNoMoreLODs_ConfigStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

