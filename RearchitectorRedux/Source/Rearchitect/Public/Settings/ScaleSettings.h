#pragma once
#include "VectorAxisLock.h"

#include "ScaleSettings.generated.h"


USTRUCT(Blueprintable, BlueprintType)
struct FArchitectorTargetScale
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, SaveGame)
	double ScaleFactor = 0.1;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	FVectorAxisLock AxisLock;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool UsePivot = false;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool UseOriginAsPivot = false;
	
	FVector TransformVector(const FVector& ScaleAxis) const { return AxisLock.ApplyLock(ScaleAxis * ScaleFactor); }
};
