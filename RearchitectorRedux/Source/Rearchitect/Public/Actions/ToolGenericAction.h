// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolActionBase.h"
#include "ToolGenericAction.generated.h"

/**
 * 
 */
UCLASS()
class REARCHITECT_API UToolGenericAction : public UToolActionBase
{
	GENERATED_BODY()

public:

	virtual void PerformAction_Implementation() override { GetRCO()->ApplyModifyDataOnEach(Data); }

	UPROPERTY() TArray<FTargetModifyData> Data;
};
