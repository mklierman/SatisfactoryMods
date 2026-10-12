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

protected:
	virtual void UpdatePoleMesh() override;
	virtual void ConfigureActor(AFGBuildable* inBuildable) const override;

private:
	UPROPERTY()
	TObjectPtr<UStaticMesh> mPipeStackableMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> mHypertubeStackableMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> mActiveStackableMesh;
};
