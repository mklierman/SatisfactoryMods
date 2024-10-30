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
		mDisqfualifyingText = FText::FromString("Snapping to connections with shared inputs or outputs is not supported");
	}
};


UCLASS()
class DIRECTTOSPLITTER_API USnapOnSplitterDisqualifier : public UFGConstructDisqualifier
{
	GENERATED_BODY()

	USnapOnSplitterDisqualifier()
	{
		mDisqfualifyingText = FText::FromString("Splitter Type not supported by SnapOn");
	}
};
