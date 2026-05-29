

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GlassCleaner_BPFL.generated.h"

/**
 * 
 */
UCLASS()
class GLASSCLEANER_API UGlassCleaner_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable, Category = "Utilities", meta = (DeterminesOutputType = "ParentClass"))
	static void GetAllSubclassesOfWithCDO(
		TSubclassOf<UObject> ParentClass,
		TArray<UObject*>& OutCDOs
	);

	UFUNCTION(BlueprintCallable, Category = "Utilities", meta = (DeterminesOutputType = "ParentClass"))
	static void GetAllSubclassesOf(
		TSubclassOf<UObject> ParentClass,
		TArray<UClass*>& OutClassess
	);

	UFUNCTION(BlueprintCallable, Category = "Utilities")
	static UObject* GetCDOQuiet(UClass* Class);
};
