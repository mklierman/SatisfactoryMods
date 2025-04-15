// ILikeBanas

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AI/FGAISystem.h"
#include "Buildables/FGBuildable.h"
#include "FGRailroadVehicleMovementComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Buildables/FGBuildableWidgetSign.h"


#include "CDO_BPLib.generated.h"


DECLARE_DYNAMIC_DELEGATE_OneParam(FOnWidgetCreated, UUserWidget*, Widget);
/**
 *
 */
UCLASS()
class CDO_API UCDO_BPLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


	static FOnWidgetCreated OnWidgetCreated;

	UFUNCTION(BlueprintCallable)
	static void UpdateSignElements(AFGBuildableWidgetSign* sign, FPrefabSignData prefabSignData);

	//UFUNCTION(BlueprinTCallable)
	//	static void SetLiftProperties(float stepHeight, float minHeight);

	//UFUNCTION(BlueprintCallable)
	//	static void MoveBuildable(AFGBuildable* inBuildable, FVector newLocation);


	//UFUNCTION(BlueprintCallable)
	//	static void ShowVehiclePaths(UObject* WorldContextObj, UStaticMesh* mesh);

	//UFUNCTION(BlueprintCallable)
	//	static TArray<USplineComponent*> GetAllVehiclePathSplines(UObject* WorldContextObj);
};
