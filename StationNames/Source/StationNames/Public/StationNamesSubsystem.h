

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "FGSaveInterface.h"
#include "StationNamesSubsystem.generated.h"

USTRUCT(BlueprintType)
struct STATIONNAMES_API FNameStruct
{
	GENERATED_BODY()

		FNameStruct() :
		IsEnabled(false),
		Name(""),
		CanLink(false)
	{}

	FNameStruct(bool isEnabled, FString name, bool canLink) :
		IsEnabled(isEnabled),
		Name(name),
		CanLink(canLink)
	{}
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		bool IsEnabled;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString Name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		bool CanLink;
};

/**
 *
 */
UCLASS()
class STATIONNAMES_API AStationNamesSubsystem : public AModSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()

public:
	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface

		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "StationNames", SaveGame)
		TArray<FNameStruct> Names1;

		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "StationNames", SaveGame)
		TArray<FNameStruct> Names2;

		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "StationNames", SaveGame)
		TArray<FNameStruct> Names3;

		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "StationNames", SaveGame)
		TArray<FNameStruct> Names4;
};
