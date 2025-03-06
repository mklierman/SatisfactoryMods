#pragma once
#include "VectorAxisLock.h"
#include "MovementSettings.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FArchitectorTargetMovement
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, SaveGame)
	double NudgeAmount = 80;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	FVectorAxisLock AxisLock;

	///Transforms vector from movement axis to delta vector (Nudge amount and axis locking)
	FVector TransformVector(const FVector& Move) const { return AxisLock.ApplyLock(Move * NudgeAmount); }

	///Transforms delta position (Applies nudge grid and axis locks)
	FVector TransformPosition(const FVector& Position) const { return AxisLock.ApplyLock(Position.GridSnap(NudgeAmount)); }
};
