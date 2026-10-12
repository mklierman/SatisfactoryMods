#pragma once

#include "CoreMinimal.h"
#include "Hologram/FGConveyorPoleHologram.h"
#include "CP_StackableConveyorPoleHologram.generated.h"

UCLASS()
class CONSTRUCTIONPREFERENCES_API ACP_StackableConveyorPoleHologram : public AFGConveyorPoleHologram
{
	GENERATED_BODY()

public:
	ACP_StackableConveyorPoleHologram();
	virtual void BeginPlay() override;

protected:
	virtual void UpdatePoleMesh() override;
	virtual void ConfigureActor(AFGBuildable* inBuildable) const override;

private:
	UPROPERTY()
	TObjectPtr<UStaticMesh> mStackablePoleMesh;
};
