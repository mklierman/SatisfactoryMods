


#include "CL_RCO.h"


void UCL_RCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCL_RCO, bDummy);
}

void UCL_RCO::Server_SetLimiterRate_Implementation(ACL_CounterLimiter* Limiter, float newRate)
{
	if (Limiter)
	{
		Limiter->SetThroughputLimit(newRate);
		Limiter->ForceNetUpdate();
	}
}

void UCL_RCO::Server_CalculateIPM_Implementation(ACL_CounterLimiter* Limiter)
{
	if (Limiter)
	{
		Limiter->CalculateIPM();
		Limiter->ForceNetUpdate();
	}
}