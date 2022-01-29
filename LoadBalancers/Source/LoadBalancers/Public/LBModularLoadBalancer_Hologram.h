

#pragma once

#include "FactoryGame.h"
#include "CoreMinimal.h"
#include "Hologram/FGFactoryHologram.h"
#include "LBBuild_ModularLoadBalancer.h"
#include "FGFactoryConnectionComponent.h"
#include "LBModularLoadBalancer_Hologram.generated.h"

/**
 *
 */
UCLASS()
class LOADBALANCERS_API ALBModularLoadBalancer_Hologram : public AFGFactoryHologram
{
	GENERATED_BODY()
public:

	virtual AActor* Construct(TArray< AActor* >& out_children, FNetConstructionID netConstructionID) override;
};
