#pragma once

#include "CoreMinimal.h"
#include "HypertubeDestinationTypes.generated.h"

class AFGBuildablePipeHyperJunction;
class AFGPipeHyperStart;
class UFGPipeConnectionComponentBase;

USTRUCT(BlueprintType)
struct HYPERTUBEDESTINATIONS_API FHypertubeDestinationRecord
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Hypertube Destinations")
	FGuid DestinationId;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Hypertube Destinations")
	FString DisplayName;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Hypertube Destinations")
	TObjectPtr<AFGPipeHyperStart> Entrance = nullptr;
};

USTRUCT(BlueprintType)
struct HYPERTUBEDESTINATIONS_API FHypertubeDestinationOption
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Hypertube Destinations")
	FGuid DestinationId;

	UPROPERTY(BlueprintReadWrite, Category = "Hypertube Destinations")
	FString DisplayName;
};

USTRUCT(BlueprintType)
struct HYPERTUBEDESTINATIONS_API FHypertubeRouteDecision
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Hypertube Destinations")
	TObjectPtr<AFGBuildablePipeHyperJunction> Junction = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Hypertube Destinations")
	TObjectPtr<UFGPipeConnectionComponentBase> IncomingConnection = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Hypertube Destinations")
	TObjectPtr<UFGPipeConnectionComponentBase> OutgoingConnection = nullptr;
};

USTRUCT(BlueprintType)
struct HYPERTUBEDESTINATIONS_API FHypertubeRoutePlan
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Hypertube Destinations")
	FGuid SourceId;

	UPROPERTY(BlueprintReadOnly, Category = "Hypertube Destinations")
	FGuid DestinationId;

	UPROPERTY(BlueprintReadOnly, Category = "Hypertube Destinations")
	TArray<FHypertubeRouteDecision> Decisions;

	UPROPERTY(BlueprintReadOnly, Category = "Hypertube Destinations")
	int32 CurrentDecisionIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Hypertube Destinations")
	int32 TopologyRevision = 0;

	TWeakObjectPtr<AFGBuildablePipeHyperJunction> LastRoutedJunction;
	TWeakObjectPtr<UFGPipeConnectionComponentBase> LastRoutedExit;

	bool IsValid() const
	{
		return SourceId.IsValid() && DestinationId.IsValid();
	}
};
