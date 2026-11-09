#pragma once

#include "CoreMinimal.h"

class UStruct;

/** Modal searchable picker used by the editor for native classes and structs. */
class FNativeStructPicker
{
public:
	static UStruct* Pick(const FText& Title, const TArray<UStruct*>& Candidates, bool bUseShortNames = false);
};
