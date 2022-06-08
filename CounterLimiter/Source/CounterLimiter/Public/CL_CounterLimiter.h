

#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildableConveyorAttachment.h"
#include "Buildables/FGBuildableWidgetSign.h"
#include "CL_CounterLimiter.generated.h"

/**
 *
 */
DECLARE_LOG_CATEGORY_EXTERN(CounterLimiter_Log, Display, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateDisplay, float, NewIPM);

UCLASS()
class COUNTERLIMITER_API ACL_CounterLimiter : public AFGBuildableConveyorAttachment
{
	GENERATED_BODY()
public:
	ACL_CounterLimiter();

		virtual void BeginPlay() override;
		void PostInitializeComponents() override;
		virtual bool ShouldSave_Implementation() const override;

	virtual void Factory_CollectInput_Implementation() override;
	virtual bool Factory_GrabOutput_Implementation(class UFGFactoryConnectionComponent* connection, FInventoryItem& out_item, float& out_OffsetBeyond, TSubclassOf< UFGItemDescriptor > type) override;

	float GetSecondsPerItem();

	//UFUNCTION(BlueprintImplementableEvent)
	void UpdateAttachedSigns();


	UPROPERTY(BlueprintAssignable, Replicated)
	FUpdateDisplay OnUpdateDisplay;

	UPROPERTY(BlueprintReadWrite, SaveGame, Replicated)
	float mPerMinuteLimitRate = -1.f;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool mIsFirstLoad = true;

	UPROPERTY(BlueprintReadWrite)
	UFGFactoryConnectionComponent* inputConnection;

	UPROPERTY(BlueprintReadWrite)
	UFGFactoryConnectionComponent* outputConnection;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	TArray<AFGBuildableWidgetSign*> mAttachedSigns;

	TMap<TSubclassOf<class UFGItemDescriptor>, int32> mItemCounts;

	int32 ItemCount;

	UPROPERTY(BlueprintReadWrite)
	UFGInventoryComponent* OutputStageBuffer;

	UPROPERTY(BlueprintReadWrite)
	UFGInventoryComponent* InputBuffer;

	UPROPERTY(BlueprintReadWrite, SaveGame, ReplicatedUsing = OnRep_SetDisplayIPM)
	float DisplayIPM = -1.f;

	UFUNCTION(BlueprintCallable)
	virtual void OnRep_SetDisplayIPM();

	FTimerHandle ipmTimerHandle;
	FTimerHandle ThroughputTimerHandle;

	UFUNCTION(BlueprintCallable)
	void SetThroughputLimit(float itemsPerMinute);


	UFUNCTION(BlueprintPure)
	float GetThroughputLimit();

	UFUNCTION()
	void CalculateIPM();

	UFUNCTION()
	void StageItemForOutput();
};

