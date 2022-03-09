

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

	virtual void SetCustomizationData_Native(const FFactoryCustomizationData& customizationData) override;
	virtual void ApplyCustomizationData_Native(const FFactoryCustomizationData& customizationData) override;
	void LocalSetCustomizationData_Native(const FFactoryCustomizationData& customizationData);
	void LocalApplyCustomizationData_Native(const FFactoryCustomizationData& customizationData);


	UPROPERTY(BlueprintReadWrite, SaveGame)
		TArray<UFGFactoryConnectionComponent*> InputConnections;

	//TMap<ALBBuild_ModularLoadBalancer*, UFGFactoryConnectionComponent*> ConnectionOwners;

	UPROPERTY(BlueprintReadWrite, SaveGame)
		UFGInventoryComponent* Buffer;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	TMap<ALBBuild_ModularLoadBalancer*, UFGFactoryConnectionComponent*> InputOwners;
	UPROPERTY(BlueprintReadWrite, SaveGame)
	TMap<ALBBuild_ModularLoadBalancer*, UFGFactoryConnectionComponent*> OutputOwners;

	UPROPERTY(BlueprintReadWrite, SaveGame, Replicated)
	TArray<ALBBuild_ModularLoadBalancer*> GroupModules;

	UPROPERTY(BlueprintReadWrite, SaveGame, Replicated)
		TArray<UMaterialInterface*> DefaultLoadBalancerMaterials;
	UPROPERTY(BlueprintReadWrite, SaveGame, Replicated)
		UMaterialInterface* TempLoadBalancerMaterial;
	UPROPERTY(BlueprintReadWrite, SaveGame, Replicated)
		UFGColoredInstanceMeshProxy* Mesh;

	FCriticalSection mOutputLock;
	FCriticalSection mInputLock;

	UPROPERTY(BlueprintReadWrite, SaveGame)
		TMap<UFGFactoryConnectionComponent*, int> OutputMap;
	UPROPERTY(BlueprintReadWrite, SaveGame)
		TMap<UFGFactoryConnectionComponent*, int> InputMap;


	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool OutputCleared;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	UFGFactoryConnectionComponent* MyOutputConnection;
	UPROPERTY(BlueprintReadWrite, SaveGame)
		UFGFactoryConnectionComponent* MyInputConnection;

	float mOffset;

	UPROPERTY(BlueprintReadWrite, SaveGame, Replicated)
	ALBBuild_ModularLoadBalancer* GroupLeader;

	UPROPERTY(BlueprintReadWrite, SaveGame, Replicated)
	ALBBuild_ModularLoadBalancer* SnappedBalancer;

protected:
	// Begin Factory_ interface
	virtual void Factory_CollectInput_Implementation() override;
	void CollectInput(UFGFactoryConnectionComponent* connection);
	void GiveOutput(bool& result, FInventoryItem& out_item);
	virtual bool Factory_GrabOutput_Implementation(class UFGFactoryConnectionComponent* connection, FInventoryItem& out_item, float& out_OffsetBeyond, TSubclassOf< UFGItemDescriptor > type) override;
	// End Factory_ interface

};
