// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolActionBase.h"
#include "ToolEmptyAction.generated.h"

/**
 * Represents empty action, doesn't do anything besides acting like a sort of "line breaker" (for stuff like "move to aim position")
 */
UCLASS()
class REARCHITECT_API UToolEmptyAction : public UToolActionBase
{
	GENERATED_BODY()
};
