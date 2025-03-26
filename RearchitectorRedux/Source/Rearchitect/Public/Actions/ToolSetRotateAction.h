// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolActionBase.h"
#include "ToolSetRotateAction.generated.h"

/**
 * 
 */
UCLASS()
class REARCHITECT_API UToolSetRotateAction : public UToolActionBase
{
	GENERATED_BODY()

public:

	virtual void PerformAction_Implementation() override;
	
	UPROPERTY() FQuat Value;
};
