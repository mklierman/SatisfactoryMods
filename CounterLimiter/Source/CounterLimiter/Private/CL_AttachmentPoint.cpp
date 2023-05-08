#include "CL_AttachmentPoint.h"

#pragma optimize("", off)
bool UCL_AttachmentPoint::CanAttach(const FFGAttachmentPoint& point, const FFGAttachmentPoint& targetPoint) const
{
	if (point.Type == targetPoint.Type)
	{
		return false;
	}
	return Super::CanAttach(point, targetPoint);
}

#pragma optimize("", on)