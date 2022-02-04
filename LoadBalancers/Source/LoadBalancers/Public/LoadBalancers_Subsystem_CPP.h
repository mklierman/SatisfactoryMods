

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "LBBuild_ModularLoadBalancer.h"
#include "FGSaveInterface.h"
#include "LoadBalancers_Subsystem_CPP.generated.h"

/**
 *
 */
//struct
//Primary balancer
//array of all balancers in group
USTRUCT(BlueprintType)
struct FLBBalancerGroup
{
	GENERATED_BODY()

		UPROPERTY(BlueprintReadWrite, SaveGame)
	ALBBuild_ModularLoadBalancer* PrimaryBalancer;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	TArray<ALBBuild_ModularLoadBalancer*> GroupBalancers;

	FLBBalancerGroup() : PrimaryBalancer(nullptr), GroupBalancers(TArray<ALBBuild_ModularLoadBalancer*>()) {}

	FLBBalancerGroup(ALBBuild_ModularLoadBalancer* primaryBalancer, TArray<ALBBuild_ModularLoadBalancer*> groupBalancers) : PrimaryBalancer(primaryBalancer), GroupBalancers(groupBalancers) {}
};


/**
Subsystem keeps track of balancer groups
If a balancer is built not attached to another, it becomes the primary for the group
Else it becomes a secondary and its inputs and outputs are given to the primary
Primary manages connection grabbing and pushing
*/
UCLASS()
class LOADBALANCERS_API ALoadBalancers_Subsystem_CPP : public AModSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()
public:
	//array of structs
	UPROPERTY(BlueprintReadWrite, SaveGame)
	TArray<FLBBalancerGroup> BalancerGroups;

	virtual bool ShouldSave_Implementation() const override;
};
