#include "SDRangeVisualizer.h"
#include "SDRangeMath.h"
#include "PrimitiveSceneProxy.h"
#include "PrimitiveViewRelevance.h"
#include "SceneManagement.h"

class FSDRangeSceneProxy final : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t Unique;
		return reinterpret_cast<size_t>(&Unique);
	}

	explicit FSDRangeSceneProxy(USDRangeDrawComponent* component)
		: FPrimitiveSceneProxy(component)
		, BaseDistance(component->BaseDistance)
		, MinCurveScale(component->MinCurveScale)
		, AnchorForward(component->AnchorForward)
		, HologramForward(component->HologramForward)
	{
		bWillEverBeLit = false;
	}

	virtual void GetDynamicMeshElements(
		const TArray<const FSceneView*>& views,
		const FSceneViewFamily& viewFamily,
		uint32 visibilityMap,
		FMeshElementCollector& collector) const override
	{
		if (BaseDistance <= 1.0f)
		{
			return;
		}

		int32 ringSegments = 64;
		float ringHeight = 80.0f;
		float ringThickness = 15.0f;
		FLinearColor ringColor(0.2f, 0.85f, 1.0f);
		FMatrix worldMatrix = GetLocalToWorld();

		for (int32 viewIndex = 0; viewIndex < views.Num(); ++viewIndex)
		{
			if (!(visibilityMap & (1 << viewIndex)))
			{
				continue;
			}

			auto pdi = collector.GetPDI(viewIndex);
			if (!pdi)
			{
				continue;
			}

			FVector prev = FVector::ZeroVector;
			for (int32 i = 0; i <= ringSegments; ++i)
			{
				int32 index = i % ringSegments;
				float theta = 2.0f * PI * static_cast<float>(index) / static_cast<float>(ringSegments);
				FVector dir(FMath::Cos(theta), FMath::Sin(theta), 0.0f);
				float reach = SDRangeMath::ComputeReach(dir, AnchorForward, HologramForward, BaseDistance, MinCurveScale);
				FVector current = worldMatrix.TransformPosition(dir * reach + FVector(0.0f, 0.0f, ringHeight));
				if (i > 0)
				{
					pdi->DrawLine(prev, current, ringColor, SDPG_World, ringThickness, 0.0f, true);
				}
				prev = current;
			}
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* view) const override
	{
		FPrimitiveViewRelevance result;
		result.bDrawRelevance = IsShown(view);
		result.bDynamicRelevance = true;
		result.bShadowRelevance = false;
		result.bRenderInMainPass = true;
		return result;
	}

	virtual uint32 GetMemoryFootprint() const override { return sizeof(*this); }
	virtual bool CanBeOccluded() const override { return false; }

private:
	float BaseDistance;
	float MinCurveScale;
	FVector AnchorForward;
	FVector HologramForward;
};

USDRangeDrawComponent::USDRangeDrawComponent()
{
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetCastShadow(false);
	SetCanEverAffectNavigation(false);
	Mobility = EComponentMobility::Movable;
}

void USDRangeDrawComponent::SetRange(float inBaseDistance, float inMinCurveScale, FVector inAnchorForward2D, FVector inHologramForward2D)
{
	FVector newAnchorForward = inAnchorForward2D.GetSafeNormal2D();
	FVector newHologramForward = inHologramForward2D.GetSafeNormal2D();
	if (FMath::IsNearlyEqual(BaseDistance, inBaseDistance, 1.0f) &&
		FMath::IsNearlyEqual(MinCurveScale, inMinCurveScale, 0.001f) &&
		AnchorForward.Equals(newAnchorForward, 0.001f) &&
		HologramForward.Equals(newHologramForward, 0.001f))
	{
		return;
	}

	BaseDistance = inBaseDistance;
	MinCurveScale = inMinCurveScale;
	AnchorForward = newAnchorForward;
	HologramForward = newHologramForward;
	UpdateBounds();
	MarkRenderStateDirty();
}

FPrimitiveSceneProxy* USDRangeDrawComponent::CreateSceneProxy()
{
	return new FSDRangeSceneProxy(this);
}

FBoxSphereBounds USDRangeDrawComponent::CalcBounds(const FTransform& localToWorld) const
{
	float radius = FMath::Max(BaseDistance, 100.0f);
	return FBoxSphereBounds(FVector::ZeroVector, FVector(radius), radius).TransformBy(localToWorld);
}
