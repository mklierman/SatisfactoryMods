

#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildableFactory.h"
#include "Buildables/FGBuildableConveyorAttachment.h"
#include "FGFactoryConnectionComponent.h"
#include "FGInventoryComponent.h"
#include "LoadBalancersModule.h"
#include "Misc/ScopeLock.h"
#include "Containers/Queue.h"
#include "LBBuild_ModularLoadBalancer.generated.h"

/**
 *
 */
UCLASS()
class LOADBALANCERS_API ALBBuild_ModularLoadBalancer : public AFGBuildableConveyorAttachment
{
	GENERATED_BODY()
public:

	ALBBuild_ModularLoadBalancer();
	// Begin AActor interface
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	virtual void Tick(float dt) override;
	// End AActor interface
	virtual bool ShouldSave_Implementation() const override;

	UPROPERTY(BlueprintReadWrite, SaveGame)
		TArray<UFGFactoryConnectionComponent*> InputConnections;

	//TMap<ALBBuild_ModularLoadBalancer*, UFGFactoryConnectionComponent*> ConnectionOwners;

	UPROPERTY(BlueprintReadWrite, SaveGame)
		UFGInventoryComponent* Buffer;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	TMap<ALBBuild_ModularLoadBalancer*, UFGFactoryConnectionComponent*> InputOwners;
	UPROPERTY(BlueprintReadWrite, SaveGame)
	TMap<ALBBuild_ModularLoadBalancer*, UFGFactoryConnectionComponent*> OutputOwners;

	FCriticalSection mOutputLock;
	FCriticalSection mInputLock;

	UPROPERTY(BlueprintReadWrite, SaveGame)
		TMap<UFGFactoryConnectionComponent*, int> OutputMap;


		TQueue< UFGFactoryConnectionComponent*, EQueueMode::Mpsc> InputQueue;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool OutputCleared;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	UFGFactoryConnectionComponent* MyOutputConnection;

	float mOffset;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	ALBBuild_ModularLoadBalancer* GroupLeader;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	ALBBuild_ModularLoadBalancer* SnappedBalancer;

protected:
	// Begin Factory_ interface
	virtual void Factory_CollectInput_Implementation() override;
	void GiveOutput(bool& result, FInventoryItem& out_item);
	virtual bool Factory_GrabOutput_Implementation(class UFGFactoryConnectionComponent* connection, FInventoryItem& out_item, float& out_OffsetBeyond, TSubclassOf< UFGItemDescriptor > type) override;
	// End Factory_ interface

};
