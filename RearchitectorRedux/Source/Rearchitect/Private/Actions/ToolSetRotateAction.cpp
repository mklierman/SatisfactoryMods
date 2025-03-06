// Fill out your copyright notice in the Description page of Project Settings.


#include "Actions/ToolSetRotateAction.h"

void UToolSetRotateAction::PerformAction_Implementation()
{
	auto RCO = GetRCO();

	FActorTransformModifyData TransformData;
	TransformData.Rotate = FRotationModifier(Value, true);

	RCO->ApplyModifyDataToAll(Targets, TransformData);	
}
