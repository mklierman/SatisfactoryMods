#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Multithreaded.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMultithreaded : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DESTRUCTIBLEGASPILLARS_API IMultithreaded
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Multithreaded")
		void MultithreadedFunction();
};
