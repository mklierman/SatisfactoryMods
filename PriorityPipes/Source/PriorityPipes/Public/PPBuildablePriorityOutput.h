
#pragma once

#include "CoreMinimal.h"
#include "PPBuildablePriorityRouter.h"
#include "PPBuildablePriorityOutput.generated.h"

UCLASS(Blueprintable)
class PRIORITYPIPES_API APPBuildablePriorityOutput : public APPBuildablePriorityRouter
{
	GENERATED_BODY()
public:
	APPBuildablePriorityOutput();

	UPROPERTY(BlueprintReadWrite, Category = "Priority Pipes")
	TObjectPtr<UFGPipeConnectionFactory> In;

	UPROPERTY(BlueprintReadWrite, Category = "Priority Pipes")
	TObjectPtr<UFGPipeConnectionFactory> PriorityOut;

	UPROPERTY(BlueprintReadWrite, Category = "Priority Pipes")
	TObjectPtr<UFGPipeConnectionFactory> OverflowOut;

protected:
	virtual void RouteFluids(float dt) override;
};
