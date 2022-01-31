

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

	ALBBuild_ModularLoadBalancer();
	// Begin AActor interface
	virtual void BeginPlay() override;
	// End AActor interface

	UPROPERTY(BlueprintReadWrite)
		TArray<UFGFactoryConnectionComponent*> InputConnections;

	TMap< TSubclassOf<class ALBBuild_ModularLoadBalancer*>, UFGFactoryConnectionComponent*> ConnectionOwners;

	UPROPERTY(BlueprintReadWrite)
		UFGInventoryComponent* Buffer;

	int mBufferIndex;

	UPROPERTY(BlueprintReadWrite)
		TMap<UFGFactoryConnectionComponent*, int> OutputMap;

	float mOffset;

	TSubclassOf<class ALBBuild_ModularLoadBalancer*> GroupLeader;

	TSubclassOf<class ALBBuild_ModularLoadBalancer*> SnappedBalancer;

protected:
	// Begin Factory_ interface
	virtual void Factory_CollectInput_Implementation() override;
	void GiveOutput(bool& result, FInventoryItem& out_item);
	virtual bool Factory_GrabOutput_Implementation(class UFGFactoryConnectionComponent* connection, FInventoryItem& out_item, float& out_OffsetBeyond, TSubclassOf< UFGItemDescriptor > type) override;
	// End Factory_ interface

	UFUNCTION(BlueprintCallable)
		virtual bool HandleGrabOutput(UFGFactoryConnectionComponent* OutputConnection, FInventoryItem& out_item, float& out_OffsetBeyond);

};
