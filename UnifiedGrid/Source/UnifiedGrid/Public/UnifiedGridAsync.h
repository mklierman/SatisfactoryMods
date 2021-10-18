#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UnifiedGridAsync.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UUnifiedGridAsync : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class UNIFIEDGRID_API IUnifiedGridAsync
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Async")
		void UnifiedGridAsyncFunction();
};
