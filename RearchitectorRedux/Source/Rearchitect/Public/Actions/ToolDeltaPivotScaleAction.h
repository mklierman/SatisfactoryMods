// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolActionBase.h"
#include "Settings/VectorAxisLock.h"
#include "ToolDeltaPivotScaleAction.generated.h"

/**
 * 
 */
UCLASS()
class REARCHITECT_API UToolDeltaPivotScaleAction : public UToolActionBase
{
	GENERATED_BODY()

public:

	UPROPERTY() FVector ScaleAmount;
	UPROPERTY() FVectorAxisLock MovementLock;
	UPROPERTY() FVector ShrinkCenter;


private:

	virtual void PerformAction_Implementation() override;
};
