// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolActionBase.h"
#include "ToolMoveToPositionAction.generated.h"

/**
 * 
 */
UCLASS()
class REARCHITECT_API UToolMoveToPositionAction : public UToolActionBase
{
	GENERATED_BODY()

public:

	virtual void PerformAction_Implementation() override;
	
	UPROPERTY() FVector CurrentDelta;
};
