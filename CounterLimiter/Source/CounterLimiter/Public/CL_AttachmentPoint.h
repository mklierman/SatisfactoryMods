#pragma once

#include "CoreMinimal.h"
#include "FGAttachmentPoint.h"
#include "CL_AttachmentPoint.generated.h"

UCLASS()
class COUNTERLIMITER_API UCL_AttachmentPoint : public UFGAttachmentPointType
{
	GENERATED_BODY()

public:
	//virtual bool CanAttach(const struct FFGAttachmentPoint& point, const struct FFGAttachmentPoint& targetPoint) const override;
};
