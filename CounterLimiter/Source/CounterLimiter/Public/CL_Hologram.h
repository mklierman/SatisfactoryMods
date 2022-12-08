#pragma once

#include "CoreMinimal.h"
#include "Hologram/FGConveyorAttachmentHologram.h"
#include "CL_Hologram.generated.h"

UCLASS()
class COUNTERLIMITER_API ACL_Hologram : public AFGConveyorAttachmentHologram
{
	GENERATED_BODY()
public:
	virtual void ConfigureComponents(class AFGBuildable* inBuildable) const override;
};