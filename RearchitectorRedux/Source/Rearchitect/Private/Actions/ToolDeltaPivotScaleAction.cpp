// Fill out your copyright notice in the Description page of Project Settings.


#include "Actions/ToolDeltaPivotScaleAction.h"

void UToolDeltaPivotScaleAction::PerformAction_Implementation()
{
	TArray<FTargetModifyData> TransformDatas;
	FVector DistanceModifier = ScaleAmount;
	if(DistanceModifier.X == 0) DistanceModifier.X = 1;
	if(DistanceModifier.Y == 0) DistanceModifier.Y = 1;
	if(DistanceModifier.Z == 0) DistanceModifier.Z = 1;
	
	for (const FArchitectorToolTarget& Target : Targets)
	{
		const auto TargetDistance = Target.Target->GetActorLocation() - ShrinkCenter;
		const auto ScaledDistance = TargetDistance * DistanceModifier;
		const auto DistanceDelta = TargetDistance - ScaledDistance;

		FTargetModifyData Data;
		Data.Target = Target;
		Data.TransformData.Move = FPositionModifier(MovementLock.ApplyLock(DistanceDelta));
		Data.TransformData.Scale = FScaleModifier(ScaleAmount);

		TransformDatas.Add(Data);
	}

	GetRCO()->ApplyModifyDataOnEach(TransformDatas);
}

