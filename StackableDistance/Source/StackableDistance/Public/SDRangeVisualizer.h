#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "SDRangeVisualizer.generated.h"

UCLASS()
class USDRangeDrawComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	USDRangeDrawComponent();

	void SetRange(float inBaseDistance, float inMinCurveScale, FVector inAnchorForward2D, FVector inHologramForward2D);

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& localToWorld) const override;
	virtual bool ShouldRecreateProxyOnUpdateTransform() const override { return false; }

	float BaseDistance = 0.0f;
	float MinCurveScale = 1.0f;
	FVector AnchorForward = FVector::ForwardVector;
	FVector HologramForward = FVector::ForwardVector;
};
