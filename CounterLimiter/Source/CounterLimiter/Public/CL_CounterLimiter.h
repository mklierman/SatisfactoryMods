

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
	float GetSecondsSinceLastInput();

	//UFUNCTION(BlueprintImplementableEvent)
	void UpdateAttachedSigns();


	UPROPERTY(BlueprintAssignable)
	FUpdateDisplay OnUpdateDisplay;

	UPROPERTY(BlueprintReadWrite, SaveGame, Replicated)
	float mPerMinuteLimitRate;

	UPROPERTY(BlueprintReadWrite, Replicated)
	UFGFactoryConnectionComponent* inputConnection;

	UPROPERTY(BlueprintReadWrite, Replicated)
	UFGFactoryConnectionComponent* outputConnection;

	UPROPERTY(BlueprintReadWrite, SaveGame, Replicated)
	TArray<AFGBuildableWidgetSign*> mAttachedSigns;

	TMap<TSubclassOf<class UFGItemDescriptor>, int32> mItemCounts;

	UFUNCTION(BlueprintPure)
	int32 GetCurrentPerMinuteThroughput();

	FCriticalSection mOutputLock;

	int32 ItemCount;

	UPROPERTY(BlueprintReadWrite, Replicated)
	UFGInventoryComponent* OutputStageBuffer;

	UPROPERTY(BlueprintReadWrite, Replicated)
		UFGInventoryComponent* InputBuffer;

	UPROPERTY(BlueprintReadWrite, SaveGame, Replicated)
	float DisplayIPM = -1;

	FTimerHandle ipmTimerHandle;
	FTimerHandle ThroughputTimerHandle;

	UFUNCTION(BlueprintCallable)
		void SetThroughputLimit(float itemsPerMinute);

	UFUNCTION()
		void CalculateIPM();

	UFUNCTION()
		void StageItemForOutput();

private:
	int32 mCurrentPerMinuteThroughput;
	int32 mGameSecondsAtLastInput;
};

