#pragma once

#include "Hologram/FGPipelineJunctionHologram.h"
#include "InfiniteNudge_PipeJunctionHolo.generated.h"

UCLASS()
class INFINITENUDGE_API AInfiniteNudge_PipeJunctionHolo :  public AFGPipelineJunctionHologram
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

