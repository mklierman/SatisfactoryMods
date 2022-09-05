#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "FGDropPod.h"
#include "FGMapMarker.h"
#include "CrashSiteBeaconsSubSystem.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(CrashSiteBeacons_Log, Display, All);

UCLASS()
class ACrashSiteBeaconsSubSystem : public AModSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
		void GenerateMapMarker(FString markerName, FVector_NetQuantize markerLocation);

	UFUNCTION(BlueprintCallable)
		void AddEnumType();

	UFUNCTION(BlueprintCallable)
		void HookCrashSites();

	UFUNCTION(BlueprintCallable)
		bool HasPodBeenLooted(AFGDropPod* pod);

	UFUNCTION(BlueprintCallable)
		int GetNewEnumValue();

	UPROPERTY(SaveGame, BlueprintReadWrite)
		TArray<AFGDropPod*> PodMarkers;

	UPROPERTY(BlueprintReadWrite)
		int EnumIndex;

	UPROPERTY(SaveGame, BlueprintReadWrite)
		bool HasBeenCleaned = false;
};
