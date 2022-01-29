

#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildableFactory.h"
#include "FGFactoryConnectionComponent.h"
#include "FGInventoryComponent.h"
#include "LoadBalancersModule.h"
#include "LBBuild_ModularLoadBalancer.generated.h"

/**
 *
 */
UCLASS()
class LOADBALANCERS_API ALBBuild_ModularLoadBalancer : public AFGBuildableFactory
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite)
		TArray<UFGFactoryConnectionComponent*> InputConnections;

	UPROPERTY(BlueprintReadWrite)
		UFGInventoryComponent* Buffer;

	int mBufferIndex;

	UPROPERTY(BlueprintReadWrite)
		TMap<UFGFactoryConnectionComponent*, int> OutputMap;

	float mOffset;
};
