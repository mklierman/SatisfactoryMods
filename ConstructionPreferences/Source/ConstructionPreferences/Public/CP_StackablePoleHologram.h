#pragma once

#include "CoreMinimal.h"
#include "Hologram/FGStackablePoleHologram.h"
#include "CP_StackablePoleHologram.generated.h"

UCLASS()
class CONSTRUCTIONPREFERENCES_API ACP_StackablePoleHologram : public AFGStackablePoleHologram
{
	GENERATED_BODY()

protected:
	virtual bool IsHologramIdenticalToActor(AActor* actor, const FTransform& hologramTransform) const override;
};
