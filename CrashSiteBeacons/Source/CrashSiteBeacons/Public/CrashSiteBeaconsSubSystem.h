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
		void ForceRemoveMapMarker(const FMapMarker& marker);

		UFUNCTION(BlueprintCallable)
		FVector GetMapMarkerRepresentationLocation(const FMapMarker& mapMarker);

		UFUNCTION(BlueprintCallable)
		void RemoveRepresentation(class UFGActorRepresentation* actorRepresentation);

		UFUNCTION(BlueprintCallable)
		void UpdateRepresentation(UFGActorRepresentation* actorRepresentation);

	//void OnDropPodOpen(CallScope<void(*)(AFGDropPod*)>& scope, AFGDropPod* DropPod);

	UFUNCTION(BlueprintCallable)
		void AddEnumType();

	UFUNCTION(BlueprintCallable)
		void HookCrashSites();

	UFUNCTION(BlueprintCallable)
		bool HasPodBeenLooted(AFGDropPod* pod);

	UFUNCTION(BlueprintCallable)
		int GetNewEnumValue();

	UFUNCTION(BlueprintCallable)
	void ClearAllMarkers();

	UFUNCTION(BlueprintCallable)
	void ClearPodMarkers();

	UPROPERTY(SaveGame, BlueprintReadWrite)
		TArray<AFGDropPod*> PodMarkers;

	UPROPERTY(BlueprintReadWrite)
		int EnumIndex;

	UPROPERTY(SaveGame, BlueprintReadWrite)
		bool HasBeenCleaned = false;
};
