#pragma once

#include "CoreMinimal.h"

enum class EAccessTransformerKind : uint8
{
	Friend,
	Accessor,
	BlueprintReadWrite,
	BlueprintCallable,
	EditAnywhere,
	BlueprintType
};

struct FAccessTransformerEditRow
{
	EAccessTransformerKind Kind = EAccessTransformerKind::BlueprintReadWrite;
	FString Target;
	FString Member;
	FString Comment;
	FString ValidationMessage;
	bool bValid = true;
	bool bWarning = false;
	bool bDuplicate = false;
	bool bPendingSave = false;
};
