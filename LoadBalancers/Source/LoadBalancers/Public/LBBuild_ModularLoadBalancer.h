

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
struct LOADBALANCERS_API FLBBalancerData_Filters
{
	GENERATED_BODY()

	FLBBalancerData_Filters() = default;
	FLBBalancerData_Filters(ALBBuild_ModularLoadBalancer* Balancer)
	{
		mBalancer.Add(Balancer);
		mOutputIndex = 0;
	}
	
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> mBalancer;
	
	UPROPERTY(SaveGame)
	int mOutputIndex = 0;
};

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
	TMap<TSubclassOf<UFGItemDescriptor>, int> mOutputIndex;
	
	UPROPERTY(SaveGame)
	TMap<TSubclassOf<UFGItemDescriptor>, FLBBalancerData_Filters> mFilterMap;

	int GetOutputIndexFromItem(TSubclassOf<UFGItemDescriptor> Item, bool IsFilter = false)
	{
		if(IsFilter)
		{
			if(HasItemFilterBalancer(Item))
			{
				return mFilterMap[Item].mOutputIndex;
			}
			return -1;
		}

		if(mOutputIndex.Contains(Item))
		{
			return mOutputIndex[Item];
		}
		
		mOutputIndex.Add(Item, 0);
		return 0;
	}

	void SetFilterItemForBalancer(ALBBuild_ModularLoadBalancer* Balancer, TSubclassOf<UFGItemDescriptor> Item, TSubclassOf<UFGItemDescriptor> OldItem)
	{
		if(HasItemFilterBalancer(OldItem))
		{
			RemoveBalancer(Balancer, OldItem);
		}
		
		if(HasItemFilterBalancer(Item))
		{
			mFilterMap[Item].mBalancer.AddUnique(Balancer);
			if(mFilterMap[Item].mOutputIndex == -1)
				mFilterMap[Item] = 0;
		}
		else
		{
			mFilterMap.Add(Item, FLBBalancerData_Filters(Balancer));
			if(mFilterMap[Item].mOutputIndex == -1)
				mFilterMap[Item] = 0;
		}
	}

	void RemoveBalancer(ALBBuild_ModularLoadBalancer* Balancer, TSubclassOf<UFGItemDescriptor> OldItem)
	{
		if(HasItemFilterBalancer(OldItem))
		{
			if(mFilterMap[OldItem].mBalancer.Contains(Balancer))
				mFilterMap[OldItem].mBalancer.Remove(Balancer);
			if(mFilterMap[OldItem].mBalancer.Num() == 0)
				mFilterMap.Remove(OldItem);

			if(Balancer)
			{
				if(mConnectedOutputs.Contains(Balancer))
					mConnectedOutputs.Remove(Balancer);
			}
		}
	}
	
	bool HasItemFilterBalancer(TSubclassOf<UFGItemDescriptor> Item) const
	{
		if(Item)
			return mFilterMap.Contains(Item);
		return false;
	}

	TArray<ALBBuild_ModularLoadBalancer*> GetBalancerForFilters(TSubclassOf<UFGItemDescriptor> Item)
	{
		TArray<ALBBuild_ModularLoadBalancer*> Balancers = {};

		if(HasItemFilterBalancer(Item))
		{
			for (TWeakObjectPtr<ALBBuild_ModularLoadBalancer> Balancer : mFilterMap[Item].mBalancer)
			{
				if(Balancer.IsValid())
				{
					Balancers.Add(Balancer.Get());
				}
			}
		}

		return Balancers;
	}
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

	UFUNCTION()
	void OnOutputItemRemoved( TSubclassOf<UFGItemDescriptor> itemClass, int32 numRemoved );

	/** Apply this module to the group (Called on BeginPlay should not need to call again) */
	void ApplyGroupModule();

	/** Is this module the Leader return true if yes */
	FORCEINLINE bool IsLeader() const { return GroupLeader == this; }

	UFUNCTION(BlueprintCallable, Category="Modular Loader")
	void SetFilteredItem(TSubclassOf<UFGItemDescriptor> ItemClass);

	/** Make this Loader to new GroupLeader (Tell all group modules the new leader and force an update) */
	void ApplyLeader();

	/* Get ALL valid GroupModules */
	TArray<ALBBuild_ModularLoadBalancer*> GetGroupModules() const;

	/* Overflow get pointer for the loader */
	FORCEINLINE ALBBuild_ModularLoadBalancer* GetOverflowLoader() const { return HasOverflowModule() ? GroupLeader->mOverflowModule.Get() : nullptr; }

	/* Do we have a overflow loader? (return true if pointer valid) */
	FORCEINLINE bool HasOverflowModule() const
	{
		if(GroupLeader)
			return GroupLeader->mOverflowModule.IsValid();
		return false;
	}

	FORCEINLINE bool HasFilterModule() const
	{
		if(GroupLeader)
			return GetConnections(EFactoryConnectionDirection::FCD_OUTPUT, true).Num() > 0;
		return false;
	}

	/* Some native helper */
	FORCEINLINE bool IsOverflowModule() const { return mLoaderType == ELoaderType::Overflow; }
	FORCEINLINE bool IsFilterModule() const { return mLoaderType == ELoaderType::Filter; }
	FORCEINLINE bool IsNormalModule() const { return mLoaderType == ELoaderType::Normal; }

	UPROPERTY(BlueprintReadWrite, SaveGame, Replicated)
	ALBBuild_ModularLoadBalancer* GroupLeader;

	UPROPERTY(SaveGame)
	int mLastPulledInventoryIndex = -1;

	/** Current Filtered Item (if Filter) */
	UPROPERTY(BlueprintReadOnly, Replicated, SaveGame)
	TSubclassOf<UFGItemDescriptor> mFilteredItem = UFGNoneDescriptor::StaticClass();

	// Dont need to be Saved > CAN SET BY CPP
	UPROPERTY(Transient)
	UFGFactoryConnectionComponent* MyOutputConnection;

	// Dont need to be Saved > CAN SET BY CPP
	UPROPERTY(Transient)
	UFGFactoryConnectionComponent* MyInputConnection;
	TArray<UFGFactoryConnectionComponent*> GetConnections(EFactoryConnectionDirection Direction = EFactoryConnectionDirection::FCD_OUTPUT, bool Filtered = false) const;

	/** What type is this loader? */
	UPROPERTY(EditDefaultsOnly, Category="ModularLoader")
	ELoaderType mLoaderType = ELoaderType::Normal;

private:
	/** Update our cached In and Outputs */
	void UpdateCache();

	/** Collect Logic for an Input
	 * Return true if the Item was store into a balancer
	 */
	bool CollectInput(ALBBuild_ModularLoadBalancer* Module);

	/* All Loaders */
	UPROPERTY(SaveGame)
	FLBBalancerData mNormalLoaderData;

	/* All FilteredLoaders */
	UPROPERTY(SaveGame)
	FLBBalancerData mFilteredModuleData;

	/* Overflow loader */
	UPROPERTY(Transient)
	TWeakObjectPtr<ALBBuild_ModularLoadBalancer> mOverflowModule;

	/** All our group modules */
	UPROPERTY(Replicated)
	TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> mGroupModules;

	UPROPERTY(SaveGame)
	float mTimer;
protected:
	// Begin AActor Interface
	virtual void PostInitializeComponents() override;
	// End AActor Interface

	// Begin AFGBuildableFactory interface
	virtual void Factory_Tick(float dt) override;
	virtual void Factory_CollectInput_Implementation() override;
	ALBBuild_ModularLoadBalancer* GetNextInputBalancer(FInventoryItem Item);
	// End
};