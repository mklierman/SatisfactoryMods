#pragma once

#include "CoreMinimal.h"
#include "Hologram/FGConveyorAttachmentHologram.h"
#include "CL_Hologram.generated.h"

UCLASS()
class COUNTERLIMITER_API ACL_Hologram : public AFGConveyorAttachmentHologram
{
	GENERATED_BODY()
public:
	ACL_Hologram();
	virtual void BeginPlay() override;
	virtual void SetHologramLocationAndRotation(const FHitResult& hitResult) override;
	virtual bool IsValidHitResult(const FHitResult& hitResult) const override;
	virtual void Scroll(int32 delta) override;
	virtual void ConfigureComponents(class AFGBuildable* inBuildable) const override;

private:
	int mRotationAmount = 0;
};