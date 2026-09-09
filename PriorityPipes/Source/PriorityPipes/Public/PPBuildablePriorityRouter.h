
#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildableFactory.h"
#include "FGColoredInstanceMeshProxy.h"
#include "FGInventoryComponent.h"
#include "FGPipeConnectionFactory.h"
#include "PPBuildablePriorityRouter.generated.h"

UCLASS(Abstract, Blueprintable)
class PRIORITYPIPES_API APPBuildablePriorityRouter : public AFGBuildableFactory
{
	GENERATED_BODY()
public:
	APPBuildablePriorityRouter();

	virtual void BeginPlay() override;
	virtual void Tick(float dt) override;
	virtual void Factory_Tick(float dt) override;
	virtual EProductionStatus GetProductionIndicatorStatus() const override;

	UFUNCTION(BlueprintPure, Category = "Priority Pipes")
	TArray<UFGPipeConnectionFactory*> GetPorts();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Priority Pipes")
	TObjectPtr<UFGColoredInstanceMeshProxy> Mesh;

protected:
	virtual void RouteFluids(float dt);

	UFGPipeConnectionFactory* CreatePipePort(FName name, FVector loc, FRotator rot, bool bOutput);

	int32 GetFlowLiters(UFGPipeConnectionFactory* port, float dt);
	int32 GetPullRoom(float dt);
	bool CanOutputFluid(TSubclassOf<UFGItemDescriptor> fluid);
	void ClearLeftoverIfNeeded();
	int32 PullPipe(UFGPipeConnectionFactory* port, int32 maxLiters, float dt);
	int32 PushPipe(UFGPipeConnectionFactory* port, float dt);
	void StripStrayPipePorts();
	void UpdatePortNetworks();

	UPROPERTY(SaveGame)
	FInventoryStack mBuffer;

	UPROPERTY()
	TArray<UFGPipeConnectionFactory*> mPorts;

	TArray<UFGPipeConnectionFactory*> mOutputs;
	TArray<uint8> mPortWasConnected;
};
