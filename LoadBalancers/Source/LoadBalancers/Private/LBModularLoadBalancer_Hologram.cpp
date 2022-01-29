


#include "LBModularLoadBalancer_Hologram.h"

AActor* ALBModularLoadBalancer_Hologram::Construct(TArray<AActor*>& out_children, FNetConstructionID netConstructionID)
{
	if (this->GetSnappedBuilding())
	{
		auto sb = this->GetSnappedBuilding()->GetName();
	}
	return nullptr;
}
