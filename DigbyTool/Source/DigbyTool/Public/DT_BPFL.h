#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FGColoredInstanceMeshProxy.h"
#include <FGVehicleSubsystem.h>
#include "DT_BPFL.generated.h"

UCLASS()
class DIGBYTOOL_API UDT_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	public:

		UFUNCTION(BlueprintCallable)
			static void SetMeshInstanced(UFGColoredInstanceMeshProxy* mesh, bool setToInstanced);

			UFUNCTION(BlueprintCallable)
			static void SetMeshMaterial(UFGColoredInstanceMeshProxy* mesh, int32 index, UMaterialInterface* material);

		UFUNCTION(BlueprintCallable)
		static void OpenURL(FString URL);


		UFUNCTION(BlueprintCallable)
		static void UpdateTargetList(AFGVehicleSubsystem* vehicleSubsystem, AFGDrivingTargetList* targetList);
};
