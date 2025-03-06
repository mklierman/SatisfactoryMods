
#pragma once
#include "VectorAxisLock.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FVectorAxisLock
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool X = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool Y = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool Z = false;

	FVector ApplyLock(const FVector& Vector) const
	{
		return FVector(
		X ? 0 : Vector.X,
		Y ? 0 : Vector.Y,
		Z ? 0 : Vector.Z
		);
	}
};