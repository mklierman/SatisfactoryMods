#pragma once

#include "Hologram/FGResourceExtractorHologram.h"
#include "InfiniteNudge_ResExtractorHolo.generated.h"

UCLASS()
class INFINITENUDGE_API AInfiniteNudge_ResExtractorHolo :  public AFGResourceExtractorHologram
{
	GENERATED_BODY()
public:

	virtual bool CanNudgeHologram() const override 
	{ 
		if (this->mCanNudgeHologram && this->IsHologramLocked() && this->IsDisabled() == false)
		{
			return true;
		}
		return false;
	}
};

