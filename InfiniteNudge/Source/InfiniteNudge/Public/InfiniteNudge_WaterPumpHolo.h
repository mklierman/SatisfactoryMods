#pragma once

#include "Hologram/FGWaterPumpHologram.h"
#include "InfiniteNudge_WaterPumpHolo.generated.h"

UCLASS()
class INFINITENUDGE_API AInfiniteNudge_WaterPumpHolo :  public AFGWaterPumpHologram
{
	GENERATED_BODY()
public:

	virtual bool CanNudgeHologram() const override 
	{ 
		if (this->mCanNudgeHologram && this->IsHologramLocked() && this->IsDisabled() == false)
		{
			return true;
		}
		return Super::CanNudgeHologram();
	}
};

