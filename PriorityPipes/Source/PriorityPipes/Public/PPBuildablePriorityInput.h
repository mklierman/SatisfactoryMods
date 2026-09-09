
#pragma once

#include "CoreMinimal.h"
#include "PPBuildablePriorityRouter.h"
#include "PPBuildablePriorityInput.generated.h"

UCLASS(Blueprintable)
class PRIORITYPIPES_API APPBuildablePriorityInput : public APPBuildablePriorityRouter
{
	GENERATED_BODY()
public:
	APPBuildablePriorityInput();

	UPROPERTY(BlueprintReadWrite, Category = "Priority Pipes")
	TObjectPtr<UFGPipeConnectionFactory> PriorityIn;

	UPROPERTY(BlueprintReadWrite, Category = "Priority Pipes")
	TObjectPtr<UFGPipeConnectionFactory> NormalIn;

	UPROPERTY(BlueprintReadWrite, Category = "Priority Pipes")
	TObjectPtr<UFGPipeConnectionFactory> Out;

protected:
	virtual void RouteFluids(float dt) override;
};
