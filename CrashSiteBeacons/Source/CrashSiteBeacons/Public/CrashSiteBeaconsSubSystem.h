#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "FGDropPod.h"
#include "CrashSiteBeaconsSubSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDropPodOpened, AFGDropPod*, DropPod);

UCLASS()
class ACrashSiteBeaconsSubSystem : public AModSubsystem
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable, Category = "Crash Site Beacons")
		FOnDropPodOpened OnDropPodOpened;
};
