

#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildableConveyorAttachment.h"
#include "FGFactoryConnectionComponent.h"
#include "Buildables/FGBuildableFactory.h"
#include "Buildables/FGBuildableConveyorBase.h"
#include "Replication/FGReplicationDetailInventoryComponent.h"
#include "FGInventoryComponent.h"
#include "LBBuildableBalancer3.generated.h"

/**
 *
 */
//DECLARE_LOG_CATEGORY_EXTERN(LoadBalancers_Log, Display, All);
UCLASS()
class LOADBALANCERS_API ALBBuildableBalancer3 : public AFGBuildableConveyorAttachment
{
	GENERATED_BODY()
public:
	ALBBuildableBalancer3();
	// Begin AActor interface
	virtual void BeginPlay() override;
	// End AActor interface

protected:
	// Begin Factory_ interface
	virtual void Factory_Tick(float deltaTime) override;
	virtual void Factory_CollectInput_Implementation() override;
	void GiveOutput(bool& result, FInventoryItem& out_item);
	virtual bool Factory_GrabOutput_Implementation(class UFGFactoryConnectionComponent* connection, FInventoryItem& out_item, float& out_OffsetBeyond, TSubclassOf< UFGItemDescriptor > type) override;
	// End Factory_ interface
public:

	UPROPERTY(BlueprintReadWrite)
		TArray<UFGFactoryConnectionComponent*> InputConnections;

	UPROPERTY(BlueprintReadWrite)
	UFGInventoryComponent* Buffer;

	int mBufferIndex;

	//UPROPERTY(BlueprintReadWrite)
	//int maxBufferIndex;

	UPROPERTY(BlueprintReadWrite)
	TMap<UFGFactoryConnectionComponent*, int> OutputMap;

	float mOffset;

	//UPROPERTY(BlueprintReadWrite)
	//FInventoryItem mItem;

	UFUNCTION(BlueprintCallable)
		virtual bool HandleGrabOutput(UFGFactoryConnectionComponent* OutputConnection, FInventoryItem& out_item, float& out_OffsetBeyond);

	//UFGReplicationDetailInventoryComponent mInventoryPotentialHandler;
};
