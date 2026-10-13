#include "CP_StackablePoleHologram.h"

#include "Buildables/FGBuildablePoleStackable.h"

bool ACP_StackablePoleHologram::IsHologramIdenticalToActor(AActor* actor, const FTransform& hologramTransform) const
{
	if (const AFGBuildablePoleStackable* pole = Cast<AFGBuildablePoleStackable>(actor);
		IsValid(pole) && pole->GetActorLocation().Equals(hologramTransform.GetLocation(), 10.0f))
	{
		return true;
	}

	return Super::IsHologramIdenticalToActor(actor, hologramTransform);
}
