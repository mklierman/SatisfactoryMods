#pragma once

#include "CoreMinimal.h"

namespace GraffitiPlacementMath
{
	inline bool CanDecalsOverlap(
		const FVector& Location,
		const FVector& InSurfaceNormal,
		float Size,
		const FVector& OtherLocation,
		const FVector& InOtherSurfaceNormal,
		float OtherSize,
		float NormalAlignment,
		float PlaneTolerance,
		float RadiusScale)
	{
		const FVector SurfaceNormal = InSurfaceNormal.GetSafeNormal();
		const FVector OtherSurfaceNormal = InOtherSurfaceNormal.GetSafeNormal();
		if (FVector::DotProduct(SurfaceNormal, OtherSurfaceNormal) < NormalAlignment)
		{
			return false;
		}

		const FVector Offset = OtherLocation - Location;
		const float PlaneDistance = FVector::DotProduct(Offset, SurfaceNormal);
		if (FMath::Abs(PlaneDistance) > PlaneTolerance)
		{
			return false;
		}

		const FVector TangentOffset = Offset - SurfaceNormal * PlaneDistance;
		const float CombinedRadius = (Size + OtherSize) * RadiusScale;
		return TangentOffset.SizeSquared() <= FMath::Square(CombinedRadius);
	}

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

	inline FQuat MakeDecalRotation(const FVector& InSurfaceNormal, float RollDegrees)
	{
		const FVector SurfaceNormal = InSurfaceNormal.GetSafeNormal();
		const float UpAlignment = FMath::Abs(FVector::DotProduct(FVector::UpVector, SurfaceNormal));
		const float ForwardAlignment = FMath::Abs(FVector::DotProduct(FVector::ForwardVector, SurfaceNormal));
		const FVector ReferenceAxis = UpAlignment <= ForwardAlignment ? FVector::UpVector : FVector::ForwardVector;
		FVector DecalUp = ReferenceAxis - SurfaceNormal * FVector::DotProduct(ReferenceAxis, SurfaceNormal);
		if (!DecalUp.Normalize())
		{
			DecalUp = FVector::RightVector - SurfaceNormal * FVector::DotProduct(FVector::RightVector, SurfaceNormal);
			DecalUp.Normalize();
		}

		const FQuat SurfaceRotation = FRotationMatrix::MakeFromXZ(SurfaceNormal, DecalUp).ToQuat();
		constexpr float DecalAxisCorrectionDegrees = -90.0f;
		const FQuat ExplicitRoll(SurfaceNormal, FMath::DegreesToRadians(DecalAxisCorrectionDegrees + FMath::UnwindDegrees(RollDegrees)));
		return (ExplicitRoll * SurfaceRotation).GetNormalized();
	}
}
