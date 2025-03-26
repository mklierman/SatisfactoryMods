#pragma once
#include "VectorAxisLock.h"
#include "RotationSettings.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FArchitectorTargetRotation
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, SaveGame)
	double RotateDegrees = 45;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	FVectorAxisLock AxisLock;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool UsePivot = true;

	FQuat ToDeltaRotation(const FVector& MovementAxis) const
	{
		auto AxisLocked = AxisLock.ApplyLock(MovementAxis * RotateDegrees);
		auto Angle = FMath::DegreesToRadians( AxisLocked.Dot(FVector::OneVector) );
		
		return FQuat(AxisLocked.GetSafeNormal().GetAbs(), Angle);
	}
};
