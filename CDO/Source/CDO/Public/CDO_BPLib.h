// ILikeBanas

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AI/FGAISystem.h"
#include "FGRailroadVehicleMovementComponent.h"
#include "CDO_BPLib.generated.h"


DECLARE_DYNAMIC_DELEGATE_OneParam(FOnWidgetCreated, UUserWidget*, Widget);
/**
 *
 */
UCLASS()
class CDO_API UCDO_BPLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


	static FOnWidgetCreated OnWidgetCreated;


	UFUNCTION(BlueprinTCallable)
		static void SetLiftProperties(float stepHeight, float minHeight);
};
