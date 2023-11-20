#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FGColoredInstanceMeshProxy.h"
#include "DT_BPFL.generated.h"

UCLASS()
class DIGBYTOOL_API UDT_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	public:

		UFUNCTION(BlueprintCallable)
			static void SetMeshInstanced(UFGColoredInstanceMeshProxy* mesh, bool setToInstanced);
};
