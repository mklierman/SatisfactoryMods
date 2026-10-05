#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Equipment/FGEquipment.h"
#include "FGEquipmentChild.h"
#include "BasherMk2_BPFL.generated.h"

UCLASS()
class BASHERMK2_API UBasherMk2_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void SetEquipmentVectorParameter(AFGEquipment* equipment, FName parameterName, FLinearColor value);

	UFUNCTION(BlueprintCallable)
	static void SetActorMeshVectorParameter(AActor* targetActor, FName parameterName, FLinearColor value);

	UFUNCTION(BlueprintCallable)
	static void SetMeshComponentVectorParameter(UMeshComponent* meshComponent, FName parameterName, FLinearColor value);

	static void UpdateMaterialMapInternal(TMap<FName, FFirstPersonMaterialArray>& inOutMaterialMap, FName parameterName, FLinearColor value, UObject* worldContext);
};
