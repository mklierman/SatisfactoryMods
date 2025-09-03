

#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildableFactory.h"
#include "Buildables/FGBuildableConveyorAttachment.h"
#include "FGFactoryConnectionComponent.h"
#include "FGInventoryComponent.h"
#include "Resources/FGNoneDescriptor.h"
#include "Buildables/FGBuildableFactory.h"
#include "LBBuild_ModularLoadBalancer.generated.h"

UENUM(BlueprintType)
enum class ELoaderType : uint8
{
	Normal = 0 UMETA(DisplayName = "Normal"),
	Overflow = 1 UMETA(DisplayName = "Overflow"),
	Filter = 2 UMETA(DisplayName = "Filter"),
	Programmable = 3 UMETA(DisplayName = "Programmable")
};

class ALBBuild_ModularLoadBalancer;
USTRUCT(BlueprintType)
struct LOADBALANCERS_API FLBBalancerData_Filters
{
	GENERATED_BODY()

	FLBBalancerData_Filters() = default;
	FLBBalancerData_Filters(ALBBuild_ModularLoadBalancer* Balancer)
	{
		mBalancer.Add(Balancer);
	}
	
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> mBalancer;
};

class ALBBuild_ModularLoadBalancer;
USTRUCT()
struct LOADBALANCERS_API FLBBalancerIndexing
{
	GENERATED_BODY()
	FORCEINLINE FLBBalancerIndexing(int32 InNormalIndex = -1, int32 InOverflowIndex = -1, int32 InFilterIndex = -1) :
	  mNormalIndex(InNormalIndex),
	  mOverflowIndex(InOverflowIndex),
	  mFilterIndex(InFilterIndex) {}

	int32 mNormalIndex;
	int32 mOverflowIndex;
	int32 mFilterIndex;

	// for reduce the save count
	bool Serialize( FArchive& ar )
	{
		ar << mNormalIndex;
		ar << mOverflowIndex;
		ar << mFilterIndex;
		
		return true;
	}
};

template<>
struct TStructOpsTypeTraits< FLBBalancerIndexing > : public TStructOpsTypeTraitsBase2< FLBBalancerIndexing >
{
	enum
	{
		WithSerializer = true,
		WithCopy = true
	};
};


USTRUCT(BlueprintType)
struct LOADBALANCERS_API FLBBalancerData
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> mConnectedOutputs;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> mConnectedInputs;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> mOverflowBalancer;
	
	UPROPERTY(Transient)
	TMap<TSubclassOf<UFGItemDescriptor>, FLBBalancerData_Filters> mFilterMap;

	UPROPERTY(SaveGame)
	TMap<TSubclassOf<UFGItemDescriptor>, FLBBalancerIndexing> mIndexMapping;

	UPROPERTY(SaveGame)
	int32 mInputIndex;

	UPROPERTY(SaveGame)
		TMap<TSubclassOf<UFGItemDescriptor>, int32> mFilterInputMap;

	void GetInputBalancers(TArray<ALBBuild_ModularLoadBalancer*>& Out);

	bool HasAnyValidFilter() const;

	bool HasAnyValidOverflow() const;

	void SetFilterItemForBalancer(ALBBuild_ModularLoadBalancer* Balancer, TSubclassOf<UFGItemDescriptor> Item);

	void RemoveBalancer(ALBBuild_ModularLoadBalancer* Balancer, TSubclassOf<UFGItemDescriptor> OldItem = nullptr);

	bool HasItemFilterBalancer(TSubclassOf<UFGItemDescriptor> Item) const;
};

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
	virtual void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// End AActor interface

	virtual void Dismantle_Implementation() override;

	//~ Begin IFGColorInterface
	virtual void SetCustomizationData_Native(const FFactoryCustomizationData& customizationData, bool skipCombine = false);
	virtual void StartIsAimedAtForColor_Implementation(class AFGCharacterPlayer* byCharacter, bool isValid = true);
	virtual void StopIsAimedAtForColor_Implementation(class AFGCharacterPlayer* byCharacter);
	//~ End IFGColorInterface
	virtual bool ShouldSave_Implementation() const override;

	virtual EProductionStatus GetProductionIndicatorStatus() const override;

	int32 GetNumItems(UFGInventoryComponent* bufferInventory, TSubclassOf< UFGItemDescriptor > itemClass) const;

	void SetupInventory();
	UFGInventoryComponent* GetBufferInventory();

	void InitializeModule();

	UFUNCTION()
	void OnOutputItemRemoved( TSubclassOf<UFGItemDescriptor> itemClass, int32 numRemoved, UFGInventoryComponent* targetInventory);

	/** Apply this module to the group (Called on BeginPlay should not need to call again) */
	void ApplyGroupModule();

	void RemoveGroupModule();

	/** Is this module the Leader return true if yes */
	UFUNCTION(BlueprintCallable, Category = "Modular Loader")
	FORCEINLINE bool IsLeader() const { return GroupLeader == this; }

	UFUNCTION(BlueprintCallable, Category="Modular Loader")
	void SetFilteredItem(TSubclassOf<UFGItemDescriptor> ItemClass);

	UFUNCTION(BlueprintCallable, Category="Modular Loader")
	void RemoveFilteredItem(TSubclassOf<UFGItemDescriptor> ItemClass);

	UFUNCTION(BlueprintCallable, Category = "Modular Loader")
	void SetFilteredItems(TArray<TSubclassOf<UFGItemDescriptor>> Items);

	UFUNCTION(BlueprintCallable, Category = "Modular Loader")
	void RemovedFilteredItems(TArray<TSubclassOf<UFGItemDescriptor>> Items);

	UFUNCTION(BlueprintCallable, Category = "Modular Loader")
	void RemoveAllFilteredItems();

	/** Make this Loader to new GroupLeader (Tell all group modules the new leader and force an update) */
	void ApplyLeader();

	/* Get ALL valid GroupModules */
	UFUNCTION(BlueprintCallable, Category = "Modular Loader")
	TArray<ALBBuild_ModularLoadBalancer*> GetGroupModules() const;

	/* Try to send it on the next OverflowBalancer */
	bool SendToOverflowBalancer(FInventoryItem Item) const;

	/* Try to send it on the next FilterBalancer */
	bool SendToFilterBalancer(FInventoryItem Item) const;

	/* Try to send it on the next NormalBalancer */
	bool SendToNormalBalancer(FInventoryItem Item) const;

	void CheckWeakArray(TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> weakObjects);

	/* Do we have a overflow loader? (return true if pointer valid) */
	FORCEINLINE bool HasOverflowModule() const
	{
		if (GroupLeader)
		{
			return GroupLeader->mNormalLoaderData.HasAnyValidOverflow();
		}
		return false;
	}

	FORCEINLINE bool HasFilterModule() const
	{
		if (GroupLeader)
		{
			return GroupLeader->mNormalLoaderData.HasAnyValidFilter();
		}
		return false;
	}

	/* Some native helper */
	FORCEINLINE bool IsOverflowModule() const { return mLoaderType == ELoaderType::Overflow; }
	FORCEINLINE bool IsFilterModule() const { return mLoaderType == ELoaderType::Filter || mLoaderType == ELoaderType::Programmable; }
	FORCEINLINE bool IsNormalModule() const { return mLoaderType == ELoaderType::Normal; }

	UPROPERTY(BlueprintReadWrite, SaveGame, Replicated)
	ALBBuild_ModularLoadBalancer* GroupLeader;

	/** Current Filtered Item (if Filter) */
	UPROPERTY(BlueprintReadOnly, BlueprintReadOnly, Replicated, SaveGame)
	TArray<TSubclassOf<UFGItemDescriptor>> mFilteredItems = { UFGNoneDescriptor::StaticClass() };
	
	UPROPERTY(Transient)
	TSubclassOf<UFGItemDescriptor> mFilteredItem = { UFGNoneDescriptor::StaticClass() };

	// Dont need to be Saved > CAN SET BY CPP
	UPROPERTY(Transient)
	UFGFactoryConnectionComponent* MyOutputConnection;

	// Dont need to be Saved > CAN SET BY CPP
	UPROPERTY(Transient)
	UFGFactoryConnectionComponent* MyInputConnection;

	UPROPERTY(SaveGame)
	UFGInventoryComponent* mBufferInventory;

	/** What type is this loader? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ModularLoader")
	ELoaderType mLoaderType = ELoaderType::Normal;
	
	UPROPERTY(EditDefaultsOnly, Category="ModularLoader")
	uint8 mSlotsInBuffer = 25;
	
	UPROPERTY(EditDefaultsOnly, Category="ModularLoader")
	uint8 mOverwriteSlotSize = 15;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ModularLoader")
	uint8 mMaxFilterableItems = 1;

	// Execute Logic by Leader > Factory_CollectInput_Implementation
	bool Balancer_CollectInput();

	/* All Loaders */
	UPROPERTY(SaveGame)
	FLBBalancerData mNormalLoaderData;

	/** All our group modules */
	UPROPERTY(Replicated)
	TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> mGroupModules;

private:
	/** Update our cached In and Outputs */
	void UpdateCache();

	/** Collect Logic for an Input
	 * Return true if the Item was stored into a balancer
	 */
	bool CollectInput(ALBBuild_ModularLoadBalancer* Module);

	/** Try to collect input from filter-specific inputs when normal collection fails */
	void TryCollectFromFilterInputs(const TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>>& ConnectedInputs);

	bool isLookedAtForColor = false;

	//void ApplyCustomization(ALBBuild_ModularLoadBalancer* instigator, const FFactoryCustomizationData& customizationData);
	void SetCustomization(ALBBuild_ModularLoadBalancer* instigator, const FFactoryCustomizationData& customizationData);


	UPROPERTY(SaveGame)
	float mTimer;
protected:
	// Begin AActor Interface
	virtual void PostInitializeComponents() override;
	// End AActor Interface

	// Begin AFGBuildableFactory interface
	virtual void Factory_Tick(float dt) override;
	virtual void Factory_CollectInput_Implementation() override;
	// End
};
