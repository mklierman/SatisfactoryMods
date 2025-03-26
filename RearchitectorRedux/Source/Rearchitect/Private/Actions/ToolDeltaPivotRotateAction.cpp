// Fill out your copyright notice in the Description page of Project Settings.


#include "Actions/ToolDeltaPivotRotateAction.h"

void UToolDeltaPivotRotateAction::PerformAction_Implementation()
{
	TArray<FTargetModifyData> TransformDatas;

	for (const FArchitectorToolTarget& Target : Targets)
	{
		const auto TargetDistance = Target.Target->GetActorLocation() - Origin;
		const auto RotatedTargetDistance = TargetDistance.RotateAngleAxis(Angle, Axis);
		const auto DistanceDelta = RotatedTargetDistance - TargetDistance;

		FTargetModifyData Data;
		Data.Target = Target;
		Data.TransformData.Move = FPositionModifier(MovementLock.ApplyLock(DistanceDelta));
		Data.TransformData.Rotate = FRotationModifier(FQuat(Axis, FMath::DegreesToRadians(Angle)));

		TransformDatas.Add(Data);
	}

	GetRCO()->ApplyModifyDataOnEach(TransformDatas);
}
