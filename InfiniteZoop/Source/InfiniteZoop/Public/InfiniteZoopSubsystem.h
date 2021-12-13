#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "InfiniteZoopSubsystem.generated.h"


UCLASS()
class AInfiniteZoopSubsystem : public AModSubsystem
{
	GENERATED_BODY()
public:
	AInfiniteZoopSubsystem();
	
	UPROPERTY(BlueprintReadWrite, Category = "Zoop Subsystem")
	int currentZoopAmount = 12;
};

