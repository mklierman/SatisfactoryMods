#pragma once

#include "CoreMinimal.h"
#include "Hologram/FGPipelinePoleHologram.h"
#include "CP_StackablePipelinePoleHologram.generated.h"

UCLASS()
class CONSTRUCTIONPREFERENCES_API ACP_StackablePipelinePoleHologram : public AFGPipelinePoleHologram
{
	GENERATED_BODY()

public:
	ACP_StackablePipelinePoleHologram();
	virtual void BeginPlay() override;
	virtual bool DoMultiStepPlacement(bool isInputFromARelease) override;
	virtual void SetHologramLocationAndRotation(const FHitResult& hitResult) override;
	virtual bool TrySnapToActor(const FHitResult& hitResult) override;
	virtual bool CanAdjustHeight() const override
	{
		return false;
	}
	bool IsStackedOnPole() const
	{
		return mStackedOnPole != nullptr;
	}
	void RotateStackedPole(int32 delta);

protected:
	virtual void CheckValidFloor() override;
	virtual bool IsHologramIdenticalToActor(AActor* actor, const FTransform& hologramTransform) const override;
	virtual void UpdatePoleMesh() override;
	virtual void ConfigureActor(AFGBuildable* inBuildable) const override;

private:
	bool TrySnapToStackablePole(const FHitResult& hitResult);

	UPROPERTY()
	TObjectPtr<UStaticMesh> mPipeStackableMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> mHypertubeStackableMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> mActiveStackableMesh;

	UPROPERTY()
	TObjectPtr<class AFGBuildablePoleStackable> mStackedOnPole;

	float mStackRotationOffset = 0.0f;
};
