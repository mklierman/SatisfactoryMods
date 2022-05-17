

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "HPPR_ConfigStruct.h"
#include "HPPR_Subsystem.generated.h"

/**
 *
 */
UCLASS()
class HOVERPACKPOLERANGE_API AHPPR_Subsystem : public AModSubsystem
{
	GENERATED_BODY()
public:

	UFUNCTION()
	void Mk1Updated();
	UFUNCTION()
		void Mk2Updated();
	UFUNCTION()
		void Mk3Updated();
	UFUNCTION()
		void RailsUpdated();
	UFUNCTION()
		void EverythingElseUpdated();

	void SetConfigValues();

	UPROPERTY()
	FHPPR_ConfigStruct mModConfig;

	UPROPERTY()
	int32 mMk1Range;
	UPROPERTY()
	int32 mMk2Range;
	UPROPERTY()
	int32 mMk3Range;
	UPROPERTY()
	int32 mRailRange;
	UPROPERTY()
	int32 mElseRange;

	void Loggit(FString myString);

	UPROPERTY()
	bool debugLogging = false;
};
