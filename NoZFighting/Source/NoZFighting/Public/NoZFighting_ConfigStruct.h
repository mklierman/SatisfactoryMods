#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "NoZFighting_ConfigStruct.generated.h"

struct FNoZFighting_ConfigStruct_Foundations;
struct FNoZFighting_ConfigStruct_Walls;
struct FNoZFighting_ConfigStruct_Catwalks;
struct FNoZFighting_ConfigStruct_Beams;
struct FNoZFighting_ConfigStruct_Pillars;
struct FNoZFighting_ConfigStruct_Roofs;

USTRUCT(BlueprintType)
struct FNoZFighting_ConfigStruct_Foundations {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float FoundationX{};

    UPROPERTY(BlueprintReadWrite)
    float FoundationY{};

    UPROPERTY(BlueprintReadWrite)
    float FoundationZ{};
};

USTRUCT(BlueprintType)
struct FNoZFighting_ConfigStruct_Walls {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float WallX{};

    UPROPERTY(BlueprintReadWrite)
    float WallY{};

    UPROPERTY(BlueprintReadWrite)
    float WallZ{};
};

USTRUCT(BlueprintType)
struct FNoZFighting_ConfigStruct_Catwalks {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float CatwalkX{};

    UPROPERTY(BlueprintReadWrite)
    float CatwalkY{};

    UPROPERTY(BlueprintReadWrite)
    float CatwalkZ{};
};

USTRUCT(BlueprintType)
struct FNoZFighting_ConfigStruct_Beams {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float BeamX{};

    UPROPERTY(BlueprintReadWrite)
    float BeamY{};

    UPROPERTY(BlueprintReadWrite)
    float BeamZ{};
};

USTRUCT(BlueprintType)
struct FNoZFighting_ConfigStruct_Pillars {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float PillarX{};

    UPROPERTY(BlueprintReadWrite)
    float PillarY{};

    UPROPERTY(BlueprintReadWrite)
    float PillarZ{};
};

USTRUCT(BlueprintType)
struct FNoZFighting_ConfigStruct_Roofs {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float RoofX{};

    UPROPERTY(BlueprintReadWrite)
    float RoofY{};

    UPROPERTY(BlueprintReadWrite)
    float RoofZ{};
};

/* Struct generated from Mod Configuration Asset '/NoZFighting/NoZFighting_Config' */
USTRUCT(BlueprintType)
struct FNoZFighting_ConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    FNoZFighting_ConfigStruct_Foundations Foundations{};

    UPROPERTY(BlueprintReadWrite)
    FNoZFighting_ConfigStruct_Walls Walls{};

    UPROPERTY(BlueprintReadWrite)
    FNoZFighting_ConfigStruct_Catwalks Catwalks{};

    UPROPERTY(BlueprintReadWrite)
    FNoZFighting_ConfigStruct_Beams Beams{};

    UPROPERTY(BlueprintReadWrite)
    FNoZFighting_ConfigStruct_Pillars Pillars{};

    UPROPERTY(BlueprintReadWrite)
    FNoZFighting_ConfigStruct_Roofs Roofs{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FNoZFighting_ConfigStruct GetActiveConfig(UObject* WorldContext) {
        FNoZFighting_ConfigStruct ConfigStruct{};
        FConfigId ConfigId{"NoZFighting", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FNoZFighting_ConfigStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

