#pragma once

#include "FactoryGame.h"
#include "UObject/Object.h"
#include "FGConstructDisqualifier.h"
#include "SnapOnDisqualifier.generated.h"

UCLASS()
class DIRECTTOSPLITTER_API USnapOnDisqualifier : public UFGConstructDisqualifier
{
	GENERATED_BODY()

		USnapOnDisqualifier()
	{
		mDisqfualifyingText = FText::FromString("Connecting to storage containers with multiple inputs and/or outputs is not supported");
	}
};
