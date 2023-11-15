#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "DCE_ConfigurationStruct.generated.h"

struct FDCE_ConfigurationStruct_MinSection;
struct FDCE_ConfigurationStruct_GlobalSection;
struct FDCE_ConfigurationStruct_AddSection;

USTRUCT(BlueprintType)
struct FDCE_ConfigurationStruct_MinSection {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    int32 MinLessThanInt {};

    UPROPERTY(BlueprintReadWrite)
    int32 MinSetToInt {};
};

USTRUCT(BlueprintType)
struct FDCE_ConfigurationStruct_GlobalSection {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    int32 GlobalSetToIntOld {};
};

USTRUCT(BlueprintType)
struct FDCE_ConfigurationStruct_AddSection {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    int32 AddDefaultInt {};
};

/* Struct generated from Mod Configuration Asset '/DaisyChainEverything/DCE_Configuration' */
USTRUCT(BlueprintType)
struct FDCE_ConfigurationStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    int32 ModesEnum {};

    UPROPERTY(BlueprintReadWrite)
    FDCE_ConfigurationStruct_MinSection MinSection {};

    UPROPERTY(BlueprintReadWrite)
    FDCE_ConfigurationStruct_GlobalSection GlobalSection {};

    UPROPERTY(BlueprintReadWrite)
    FDCE_ConfigurationStruct_AddSection AddSection {};

    UPROPERTY(BlueprintReadWrite)
    int32 GlobalSetToInt {};

    UPROPERTY(BlueprintReadWrite)
    bool IgnorePowerPolesBool {};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FDCE_ConfigurationStruct GetActiveConfig(UObject* WorldContext) {
        FDCE_ConfigurationStruct ConfigStruct{};
        FConfigId ConfigId{"DaisyChainEverything", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FDCE_ConfigurationStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

