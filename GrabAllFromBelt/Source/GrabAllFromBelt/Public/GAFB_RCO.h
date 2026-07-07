#pragma once

#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "FGPlayerController.h"
#include "Buildables/FGBuildableConveyorBelt.h"
#include "GAFB_RCO.generated.h"

/**
 * 
 */
UCLASS()
class GRABALLFROMBELT_API UGAFB_RCO : public UFGRemoteCallObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(Server, Reliable)
	void Server_ClearLinkedBelts(AFGBuildableConveyorBelt* belt, AFGPlayerController* controller);

	UFUNCTION(Client, Reliable)
	void Client_ClearBeltsVisual(const TArray<AFGBuildableConveyorBelt*>& belts);

	UFUNCTION(Server, Reliable)
	void Server_ClearSingleBelt(AFGBuildableConveyorBelt* belt, AFGPlayerController* controller);

private:
	void BroadcastClearBeltsVisual(const TArray<AFGBuildableConveyorBelt*>& belts);
	
};
