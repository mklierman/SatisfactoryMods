#pragma once

#include "CoreMinimal.h"

namespace GraffitiPlacementMath
{
	inline FVector SnapPointToWorldGrid(const FVector& Point, const FVector& InSurfaceNormal, float GridSize)
	{
		if (!FMath::IsFinite(GridSize) || GridSize <= 0.0f)
		{
			return Point;
		}

		const FVector Snapped(
			FMath::GridSnap(Point.X, GridSize),
			FMath::GridSnap(Point.Y, GridSize),
			FMath::GridSnap(Point.Z, GridSize));
		const FVector SurfaceNormal = InSurfaceNormal.GetSafeNormal();
		return Snapped - SurfaceNormal * FVector::DotProduct(Snapped - Point, SurfaceNormal);
	}

	inline FRotator MakeDecalRotation(const FVector& InSurfaceNormal, const FVector& InViewForward, float RollDegrees)
	{
		const FVector SurfaceNormal = InSurfaceNormal.GetSafeNormal();
		FVector DecalUp = FVector::UpVector - SurfaceNormal * FVector::DotProduct(FVector::UpVector, SurfaceNormal);
		if (!DecalUp.Normalize())
		{
			DecalUp = InViewForward - SurfaceNormal * FVector::DotProduct(InViewForward, SurfaceNormal);
			if (!DecalUp.Normalize())
			{
				DecalUp = FVector::ForwardVector;
			}
		}

		const FQuat SurfaceRotation = FRotationMatrix::MakeFromXZ(SurfaceNormal, DecalUp).ToQuat();
		constexpr float DecalAxisCorrectionDegrees = -90.0f;
		const FQuat ExplicitRoll(SurfaceNormal, FMath::DegreesToRadians(DecalAxisCorrectionDegrees + FMath::UnwindDegrees(RollDegrees)));
		return (ExplicitRoll * SurfaceRotation).Rotator();
	}
}
