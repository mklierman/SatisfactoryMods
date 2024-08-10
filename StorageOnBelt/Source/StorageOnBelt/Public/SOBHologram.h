#pragma once
#include "CoreMinimal.h"
#include "Hologram/FGConveyorAttachmentHologram.h"
#include "Buildables/FGBuildableConveyorBelt.h"
#include "Buildables/FGBuildable.h"
#include "SOBBuildableStorage.h"
#include "FGFactoryConnectionComponent.h"
#include "Buildables/FGBuildableStorage.h"
#include "SOBHologram.generated.h"

UCLASS()
class ASOBHologram : public AFGConveyorAttachmentHologram
{
	GENERATED_BODY()
public:
	virtual void SetHologramLocationAndRotation(const FHitResult& hitResult) override;
	virtual bool IsValidHitResult(const FHitResult& hitResult) const override;
	virtual void Scroll(int32 delta) override;
	virtual void ConfigureComponents(class AFGBuildable* inBuildable) const override;

	int mRotationAmount = 0;
};

