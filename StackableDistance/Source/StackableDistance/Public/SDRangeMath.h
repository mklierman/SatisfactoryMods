#pragma once

#include "CoreMinimal.h"

namespace SDRangeMath
{
	inline float ComputeReach(FVector dir2D, FVector anchorForward2D, FVector hologramForward2D, float baseDistance, float minCurveScale)
	{
		FVector dir = dir2D.GetSafeNormal2D();
		FVector anchorForward = anchorForward2D.GetSafeNormal2D();
		FVector hologramForward = hologramForward2D.GetSafeNormal2D();

		float dotAnchor = FMath::Abs(FVector::DotProduct(dir, anchorForward));
		float dotTarget = FMath::Abs(FVector::DotProduct(dir, hologramForward));
		float alignmentFactor = minCurveScale + (1.0f - minCurveScale) * (dotAnchor * dotTarget);
		return baseDistance * FMath::Clamp(alignmentFactor, minCurveScale, 1.0f);
	}
}
