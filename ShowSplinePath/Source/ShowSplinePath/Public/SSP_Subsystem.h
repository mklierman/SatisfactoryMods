#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "WheeledVehicles/FGTargetPointLinkedList.h"
#include "WheeledVehicles/FGTargetPoint.h"
#include "WheeledVehicles/FGWheeledVehicle.h"
#include "Components/SplineMeshComponent.h"
#include "FGManta.h"

#include "SSP_Subsystem.generated.h"

// Forward declarations
class USSP_RemoteCallObject;


UCLASS()
class SHOWSPLINEPATH_API ASSP_Subsystem : public AModSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()

public:
	ASSP_Subsystem();

	bool GetHasAuthority();

	virtual bool ShouldSave_Implementation() const override;
	
	UFUNCTION(BlueprintCallable)
	void HandlePathSplines(AFGDrivingTargetList* targetList, bool show);

	// Helper function to get the RCO
	class USSP_RemoteCallObject* GetRCO();

	UFUNCTION(BlueprintCallable)
	virtual void ShowTargetPath(AFGTargetPoint* targetPoint);

	UFUNCTION(BlueprintCallable)
	virtual void HideTargetPath(AFGTargetPoint* targetPoint);

	UFUNCTION(BlueprintCallable)
	virtual void ToggleVehiclePath(AFGWheeledVehicle* vehicle);

	UFUNCTION(BlueprintCallable)
	void ShowInitialPaths(AActor* actor);

	UFUNCTION(BlueprintCallable)
	void ShowAllMantaPaths(AActor* actor);
	
	UFUNCTION(BlueprintCallable)
	void HideAllMantaPaths(AActor* actor);

	UFUNCTION(BlueprintCallable)
	void ShowAllVehiclePaths(AActor* actor);
	
	UFUNCTION(BlueprintCallable)
	void HideAllVehiclePaths(AActor* actor);





	UPROPERTY(BlueprintReadWrite)
	UStaticMesh* splinePathMesh;

	UPROPERTY(BlueprintReadWrite)
	UStaticMesh* mantaPathMesh;

	// Store created spline meshes for cleanup
	UPROPERTY(BlueprintReadWrite)
	TArray<USplineMeshComponent*> AllCreatedMeshes;

	// Separate arrays for different path types
	UPROPERTY(BlueprintReadWrite)
	TArray<USplineMeshComponent*> VehicleMeshes;

	UPROPERTY(BlueprintReadWrite)
	TArray<USplineMeshComponent*> MantaMeshes;

private:
	// Flag to prevent recursion when calling SetPathVisible
	bool bIsUpdatingPathVisibility = false;

};
