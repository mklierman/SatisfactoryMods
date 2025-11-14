

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WheeledVehicles/FGWheeledVehicleMovementComponent.h"
#include <FGWheeledVehicle.h>
#include "SirenSpeed_BPFL.generated.h"

/**
 * 
 */
UCLASS()
class SIRENSPEED_API USirenSpeed_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable)
	static void SetEngineSpeed(UFGWheeledVehicleMovementComponent* movementComp, float newRPM, float newTorque);
};
