

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "LBBuild_ModularLoadBalancer.h"
#include "LoadBalancers_Subsystem_CPP.generated.h"

/**
 *
 */
//struct
//Primary balancer
//array of all balancers in group
USTRUCT()
struct FLBBalancerGroup
{
	GENERATED_BODY()

		UPROPERTY()
		TSubclassOf<class ALBBuild_ModularLoadBalancer> PrimaryBalancer;

	UPROPERTY()
	TArray< TSubclassOf<class ALBBuild_ModularLoadBalancer>> GroupBalancers;
};


/**
Subsystem keeps track of balancer groups
If a balancer is built not attached to another, it becomes the primary for the group
Else it becomes a secondary and its inputs and outputs are given to the primary
Primary manages connection grabbing and pushing
*/
UCLASS()
class LOADBALANCERS_API ALoadBalancers_Subsystem_CPP : public AModSubsystem
{
	GENERATED_BODY()
public:
	//array of structs
	UPROPERTY()
	TArray<FLBBalancerGroup> BalancerGroups;
};
