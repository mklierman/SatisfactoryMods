// Fill out your copyright notice in the Description page of Project Settings.


#include "Actions/ToolMoveToPositionAction.h"

void UToolMoveToPositionAction::PerformAction_Implementation()
{
	auto RCO = GetRCO();

	FActorTransformModifyData TransformData;
	TransformData.Move = FPositionModifier(CurrentDelta);

	RCO->ApplyModifyDataToAll(Targets, TransformData);
}
