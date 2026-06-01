#pragma once

#include "Buildables/FGBuildableStorage.h"
#include "Buildables/FGBuildableConveyorBelt.h"
#include "FGFactoryConnectionComponent.h"
#include "SOBBuildableStorage.generated.h"

UCLASS()
class ASOBBuildableStorage : public AFGBuildableStorage
{
	GENERATED_BODY()
public:
	virtual void Dismantle_Implementation() override;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UFGFactoryConnectionComponent> inputConnection;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UFGFactoryConnectionComponent> outputConnection;
};

