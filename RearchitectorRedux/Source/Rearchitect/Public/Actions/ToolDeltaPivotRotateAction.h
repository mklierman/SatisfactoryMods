// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolActionBase.h"
#include "Settings/VectorAxisLock.h"
#include "ToolDeltaPivotRotateAction.generated.h"

/**
 * 
 */
UCLASS()
class REARCHITECT_API UToolDeltaPivotRotateAction : public UToolActionBase
{
	GENERATED_BODY()



public:
	UPROPERTY() FVector Origin;
	UPROPERTY() double Angle;
	UPROPERTY() FVector Axis;
	UPROPERTY() FVectorAxisLock MovementLock;

private:

	virtual void PerformAction_Implementation() override;
};
