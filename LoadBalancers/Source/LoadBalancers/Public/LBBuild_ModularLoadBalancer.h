

#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildableFactory.h"
#include "Buildables/FGBuildableConveyorAttachment.h"
#include "FGFactoryConnectionComponent.h"
#include "FGInventoryComponent.h"
#include "Resources/FGNoneDescriptor.h"
#include "LBBuild_ModularLoadBalancer.generated.h"

UENUM(BlueprintType)
enum class ELoaderType : uint8
{
	Normal = 0 UMETA(DisplayName = "Normal"),
	Overflow = 1 UMETA(DisplayName = "Overflow"),
	Filter = 2 UMETA(DisplayName = "Filter")
};

class ALBBuild_ModularLoadBalancer;
USTRUCT()
struct LOADBALANCERS_API FLBBalancerData
{
	GENERATED_BODY()
	
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> mConnectedOutputs;
	
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> mConnectedInputs;
	
	UPROPERTY(SaveGame)
	int mInputIndex = 0;

	UPROPERTY(SaveGame)
	int mOutputIndex = 0;
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
	virtual void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// End AActor interface
	
	// Begin AFGBuildableFactory interface
	virtual void Factory_Tick(float dt) override;
	// End AFGBuildableFactory interface

	/** Apply this module to the group (Called on BeginPlay should not need to call again) */
	void ApplyGroupModule();

	/** Is this module the Leader return true if yes */
	FORCEINLINE bool IsLeader() const { return GroupLeader == this; }

	/** Should this current item sorted? */
	bool IsMarkToFilter() const;

	UFUNCTION(BlueprintCallable, Category="Modular Loader")
	void SetFilteredItem(TSubclassOf<UFGItemDescriptor> ItemClass);

	/** Make this Loader to new GroupLeader (Tell all group modules the new leader and force an update) */
	void ApplyLeader();

	/** Tick FLBBalancerData and return if something was Pushed (used for Filter and normal) */
	bool TickGroupTypeOutput(FLBBalancerData& TypeData, float dt);

	/** Try to send Items to an Output return false if the inventory full or invalid to push */
	bool SendItemsToOutputs(float dt, ALBBuild_ModularLoadBalancer* Balancer);
	
	/** Can we send something in the overflow inventory? */
	bool CanSendToOverflow() const;
	
	/* Get ALL valid GroupModules */
	TArray<ALBBuild_ModularLoadBalancer*> GetGroupModules() const;

	/* Overflow loader */
	TWeakObjectPtr<ALBBuild_ModularLoadBalancer> mOverflowLoader;
	
	/* Overflow get pointer for the loader */
	FORCEINLINE ALBBuild_ModularLoadBalancer* GetOverflowLoader() const { return HasOverflowLoader() ? GroupLeader->mOverflowLoader.Get() : nullptr; }
	
	/* Do we have a overflow loader? (return true if pointer valid) */
	FORCEINLINE bool HasOverflowLoader() const
	{
		if(GroupLeader)
			return GroupLeader->mOverflowLoader.IsValid();
		return false;
	}
	
	FORCEINLINE bool HasFilterLoader() const
	{
		if(GroupLeader)
			return GetConnections(EFactoryConnectionDirection::FCD_OUTPUT, true).Num() > 0;
		return false;
	}
	
	/* Some native helper */
	FORCEINLINE bool IsOverflowLoader() const { return mLoaderType == ELoaderType::Overflow; }
	FORCEINLINE bool IsFilterLoader() const { return mLoaderType == ELoaderType::Filter; }
	FORCEINLINE bool IsNormalLoader() const { return mLoaderType == ELoaderType::Normal; }

	UPROPERTY(BlueprintReadWrite, SaveGame, Replicated)
	ALBBuild_ModularLoadBalancer* GroupLeader;
	
	/** Current Filtered Item (if Filter) */
	UPROPERTY(BlueprintReadOnly, Replicated, SaveGame)
	TSubclassOf<UFGItemDescriptor> mFilteredItem = UFGNoneDescriptor::StaticClass();
	
	// Dont need to be Saved > CAN SET BY CPP
	UPROPERTY(Transient)
	UFGFactoryConnectionComponent* MyOutputConnection;
	
	// Dont need to be Saved > CAN SET BY CPP
	UPROPERTY(Transient)
	UFGFactoryConnectionComponent* MyInputConnection;

	// Inventory for master handle (Push items to the Modules)
	UPROPERTY(SaveGame)
	UFGInventoryComponent* mOutputInventory;
	TArray<UFGFactoryConnectionComponent*> GetConnections(EFactoryConnectionDirection Direction = EFactoryConnectionDirection::FCD_OUTPUT, bool Filtered = false) const;
	
	/** What type is this loader? */
	UPROPERTY(EditDefaultsOnly, Category="ModularLoader")
	ELoaderType mLoaderType = ELoaderType::Normal;
	
private:
	/** Update our cached In and Outputs */
	void UpdateCache();
	
	/* Timer for cache update*/
	float mTimer;

	// All Loaders
	UPROPERTY(SaveGame)
	FLBBalancerData mNormalLoaderData;
	
	// All FilteredLoaders
	UPROPERTY(SaveGame)
	FLBBalancerData mFilteredLoaderData;
	
	/** All our group modules */
	UPROPERTY(Replicated)
	TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> mGroupModules;
protected:
	// Begin AActor Interface
	virtual void PostInitializeComponents() override;
	// End AActor Interface
	
	// Begin Factory_ interface
	virtual void Factory_CollectInput_Implementation() override;
	void CollectInput(ALBBuild_ModularLoadBalancer* Module);
	// End Factory_ interface
};
