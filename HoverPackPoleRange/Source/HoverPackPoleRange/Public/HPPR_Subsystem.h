

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
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

	void SetConfigValues();

	int32 mMk1Range;
	int32 mMk2Range;
	int32 mMk3Range;
	int32 mRailRange;
	int32 mElseRange;
};
