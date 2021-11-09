// ILikeBanas

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AI/FGAISystem.h"
#include "FGRailroadVehicleMovementComponent.h"
#include "CDO_BPLib.generated.h"


/**
 * 
 */
UCLASS()
class CDO_API UCDO_BPLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

		UFUNCTION(BlueprintCallable, Category = "CDO")
		static float GetGravitationalForce(UFGRailroadVehicleMovementComponent* actor);
	UFUNCTION(BlueprintCallable, Category = "CDO")
		static float GetResistiveForce(UFGRailroadVehicleMovementComponent* actor);

	UFUNCTION(BlueprintCallable, Category = "CDO")
		static float GetGradientForce(UFGRailroadVehicleMovementComponent* actor);
};
