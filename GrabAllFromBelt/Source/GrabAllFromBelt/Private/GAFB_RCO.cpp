#include "GAFB_RCO.h"
#include "Engine/World.h"
#include "GAFB_BPFL.h"

void UGAFB_RCO::Server_ClearLinkedBelts_Implementation(AFGBuildableConveyorBelt* belt, AFGPlayerController* controller)
{
	if (!belt || !controller)
	{
		return;
	}

	TArray<AFGBuildableConveyorBelt*> linkedBelts;
	UGAFB_BPFL::ClearLinkedBeltsAndCollect(belt, controller, linkedBelts);

	BroadcastClearBeltsVisual(linkedBelts);
}

void UGAFB_RCO::Client_ClearBeltsVisual_Implementation(const TArray<AFGBuildableConveyorBelt*>& belts)
{
	for (AFGBuildableConveyorBelt* belt : belts)
	{
		UGAFB_BPFL::ClearBeltVisual(belt);
	}
}

void UGAFB_RCO::BroadcastClearBeltsVisual(const TArray<AFGBuildableConveyorBelt*>& belts)
{
	UWorld* world = GetWorld();
	if (!world)
	{
		return;
	}

	for (FConstPlayerControllerIterator iterator = world->GetPlayerControllerIterator(); iterator; ++iterator)
	{
		AFGPlayerController* playerController = Cast<AFGPlayerController>(iterator->Get());
		if (!playerController)
		{
			continue;
		}

		UGAFB_RCO* rco = playerController->GetRemoteCallObjectOfClass<UGAFB_RCO>();
		if (rco)
		{
			rco->Client_ClearBeltsVisual(belts);
		}
	}
}

void UGAFB_RCO::Server_ClearSingleBelt_Implementation(AFGBuildableConveyorBelt* belt, AFGPlayerController* controller)
{
	if (!belt || !controller)
	{
		return;
	}

	UGAFB_BPFL::ClearBelt(belt, controller);

	TArray<AFGBuildableConveyorBelt*> belts;
	belts.Add(belt);

	BroadcastClearBeltsVisual(belts);
}
