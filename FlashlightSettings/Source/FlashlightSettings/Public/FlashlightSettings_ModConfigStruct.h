#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "FlashlightSettings_ModConfigStruct.generated.h"

struct FFlashlightSettings_ModConfigStruct_SavedSettings;

USTRUCT(BlueprintType)
struct FFlashlightSettings_ModConfigStruct_SavedSettings {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float Intensity{};

    UPROPERTY(BlueprintReadWrite)
    float Attenuation{};

    UPROPERTY(BlueprintReadWrite)
    float InnerCone{};

    UPROPERTY(BlueprintReadWrite)
    float OuterCone{};

    UPROPERTY(BlueprintReadWrite)
    bool Shadows{};

    UPROPERTY(BlueprintReadWrite)
    int32 Temperature{};

    UPROPERTY(BlueprintReadWrite)
    float SpecularScale{};

    UPROPERTY(BlueprintReadWrite)
    float LightFalloff{};
};

/* Struct generated from Mod Configuration Asset '/FlashlightSettings/FlashlightSettings_ModConfig' */
USTRUCT(BlueprintType)
struct FFlashlightSettings_ModConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    bool KeepStateAfterHyperTube{};

    UPROPERTY(BlueprintReadWrite)
    FFlashlightSettings_ModConfigStruct_SavedSettings SavedSettings{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FFlashlightSettings_ModConfigStruct GetActiveConfig(UObject* WorldContext) {
        FFlashlightSettings_ModConfigStruct ConfigStruct{};
        FConfigId ConfigId{"FlashlightSettings", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FFlashlightSettings_ModConfigStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

