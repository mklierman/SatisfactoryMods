#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FGManta.h"

#include "MantaRaise_BPFL.generated.h"


USTRUCT()
struct MANTARAISE_API FMantaPoint_Struct
{
	GENERATED_BODY()
		FMantaPoint_Struct() {};
		FMantaPoint_Struct(int pId, FVector pL, double nZ)
	{
		pointId = pId;
		pointLocation = pL;
		newZ = nZ;
	}

		int pointId;
	FVector pointLocation;
	double newZ;
};

UCLASS()
class MANTARAISE_API UMantaRaise_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	public:
	UFUNCTION(BlueprintCallable)
	static void RaiseMantas(AActor* actor);

	static TArray<FVector> GetNewSplinePoints(int currentPointIndex, TArray<FVector> existingPoints, USplineComponent* spline);
	static double GetRaisedZ(TArray<FVector> points, AActor* actor, int currentIdx);
};
