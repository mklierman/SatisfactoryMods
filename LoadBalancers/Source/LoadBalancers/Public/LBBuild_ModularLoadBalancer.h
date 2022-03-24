

#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildableFactory.h"
#include "Buildables/FGBuildableConveyorAttachment.h"
#include "FGFactoryConnectionComponent.h"
#include "FGInventoryComponent.h"
#include "LBBuild_ModularLoadBalancer.generated.h"

UENUM(BlueprintType)
enum class ELoaderType : uint8
{
	Normal = 0 UMETA(DisplayName = "Normal"),
	Overflow = 1 UMETA(DisplayName = "Overflow"),
	Filter = 2 UMETA(DisplayName = "Filter")
};

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

	void ApplyGroupModule();
	
	FORCEINLINE bool IsLeader() const { return GroupLeader == this; }
	void ApplyLeader();

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> mConnectedOutputs;
	TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> mConnectedInputs;
	TArray<UFGFactoryConnectionComponent*> GetConnections(EFactoryConnectionDirection Direction = EFactoryConnectionDirection::FCD_OUTPUT) const;

	UPROPERTY(Replicated)
	TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> mGroupModules;
	TArray<ALBBuild_ModularLoadBalancer*> GetGroupModules() const;
	
	TWeakObjectPtr<ALBBuild_ModularLoadBalancer> mOverflowLoader;
	FORCEINLINE ALBBuild_ModularLoadBalancer* GetOverflowLoader() const { return HasOverflowLoader() ? mOverflowLoader.Get() : nullptr; }
	FORCEINLINE bool HasOverflowLoader() const { return mOverflowLoader.IsValid(); }
	FORCEINLINE bool IsOverflowLoader() const { return mLoaderType == ELoaderType::Overflow; }

	float mTimer;

	virtual void Factory_Tick(float dt) override;
	bool SendItemsToOutputs(float dt, ALBBuild_ModularLoadBalancer* Balancer);
	bool CanSendToOverflow() const;

	//Update our caches
	void UpdateCache();

	UPROPERTY(EditDefaultsOnly, Category="ModularLoader")
	ELoaderType mLoaderType = ELoaderType::Normal;

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

	UPROPERTY(SaveGame)
	UFGInventoryComponent* mOutputInventory;

protected:
	// Begin AActor Interface
	virtual void PostInitializeComponents() override;
	// End AActor Interface
	
	// Begin Factory_ interface
	virtual void Factory_CollectInput_Implementation() override;
	void CollectInput(ALBBuild_ModularLoadBalancer* Module);
	// End Factory_ interface

};
