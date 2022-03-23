

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
	// End AActor interface

	void ApplyGroupModule(ALBBuild_ModularLoadBalancer* Balancer);
	
	FORCEINLINE bool IsLeader() const { return GroupLeader == this; }
	void ApplyLeader(ALBBuild_ModularLoadBalancer* OldLeader);

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> mConnectedOutputs;
	TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> mConnectedInputs;
	TArray<UFGFactoryConnectionComponent*> GetConnections(EFactoryConnectionDirection Direction = EFactoryConnectionDirection::FCD_OUTPUT) const;

	UPROPERTY(Replicated)
	TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> mGroupModules;
	TArray<ALBBuild_ModularLoadBalancer*> GetGroupModules() const;

	float mTimer;

	virtual void Factory_Tick(float dt) override;

	//Update our caches
	void UpdateCache();

	FCriticalSection mOutputLock;
	FCriticalSection mInputLock;

	UPROPERTY(SaveGame)
	int mInputIndex = 0;

	UPROPERTY(SaveGame)
	int mOutputIndex = 0;

	float mOffset;

	UPROPERTY(BlueprintReadWrite, SaveGame, Replicated)
	ALBBuild_ModularLoadBalancer* GroupLeader;
	
	// Dont need to be Saved > CAN SET BY CPP
	UPROPERTY(Transient)
	UFGFactoryConnectionComponent* MyOutputConnection;
	
	// Dont need to be Saved > CAN SET BY CPP
	UPROPERTY(Transient)
	UFGFactoryConnectionComponent* MyInputConnection;

protected:
	// Begin AActor Interface
	virtual void PostInitializeComponents() override;
	// End AActor Interface
	
	// Begin Factory_ interface
	virtual void Factory_CollectInput_Implementation() override;
	void CollectInput(ALBBuild_ModularLoadBalancer* Module);
	void PushOutput(ALBBuild_ModularLoadBalancer* Module);
	bool CanPushOutput(ALBBuild_ModularLoadBalancer* Module) const;
	virtual bool Factory_GrabOutput_Implementation(class UFGFactoryConnectionComponent* connection, FInventoryItem& out_item, float& out_OffsetBeyond, TSubclassOf< UFGItemDescriptor > type) override;
	// End Factory_ interface

};
