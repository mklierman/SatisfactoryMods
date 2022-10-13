#pragma once

#include "Equipment/FGEquipment.h"
#include "Equipment/FGEquipmentAttachment.h"
#include "WheeledVehicles/FGTargetPointLinkedList.h"
#include "WheeledVehicles/FGTargetPoint.h"
#include "WheeledVehicles/FGWheeledVehicle.h"
#include "Components/SplineMeshComponent.h"
#include "FGPlayerController.h"
#include "VPT_Equip.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(VPT_Log, Display, All);

UENUM(BlueprintType)
enum class EVPT_Mode : uint8
{
	VPTM_All_ShowHide,
	VPTM_SingleNode_ShowHide,
	VPTM_SingleNode_EditDelete,
	VPTM_SingleNode_Move,
	VPTM_SingleNode_CopyPaste,
	VPTM_SingleVehicle_AI
};


UCLASS()
class VEHICLEPATHTOOL_API AVPT_Equip : public AFGEquipment
{
	GENERATED_BODY()
public:
	AVPT_Equip();

	//virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "VehiclePathTool")
	virtual void HandleHitActor(AActor* traceHitActor);

	UFUNCTION(BlueprintCallable, Category = "VehiclePathTool")
	virtual void PrimaryFirePressed(AFGPlayerController* playerController);

	UFUNCTION(BlueprintCallable, Category = "VehiclePathTool")
	virtual void SecondaryFirePressed(AFGPlayerController* playerController);

	UFUNCTION(BlueprintCallable, Category = "VehiclePathTool")
	virtual void CycleMode();

	UFUNCTION(BlueprintCallable, Category = "VehiclePathTool")
	virtual EVPT_Mode GetCurrentMode();

	void HandlePathSplines(AFGDrivingTargetList* targetList, bool show);

	UFUNCTION(BlueprintCallable, Category = "VehiclePathTool")
	virtual void ShowAllPaths();

	UFUNCTION(BlueprintCallable, Category = "VehiclePathTool")
	virtual void HideAllPaths();

	UFUNCTION(BlueprintCallable, Category = "VehiclePathTool")
	virtual void ShowTargetPath(AFGTargetPoint* targetPoint);

	UFUNCTION(BlueprintCallable, Category = "VehiclePathTool")
	virtual void HideTargetPath(AFGTargetPoint* targetPoint);

	UFUNCTION(BlueprintCallable, Category = "VehiclePathTool")
	virtual void ToggleVehiclePath(AFGWheeledVehicle* vehicle);

	UFUNCTION(BlueprintCallable, Category = "VehiclePathTool")
	virtual void EditTargetNode(AFGTargetPoint* targetPoint);

	UFUNCTION(BlueprintCallable, Category = "VehiclePathTool")
	virtual void DeleteTargetNode(AFGTargetPoint* targetPoint);

	UFUNCTION(BlueprintCallable, Category = "VehiclePathTool")
	virtual void MoveTargetNode(AFGTargetPoint* targetPoint);

	UFUNCTION(BlueprintCallable, Category = "VehiclePathTool")
	virtual void CopyTargetNodeSpeed(AFGTargetPoint* targetPoint);

	UFUNCTION(BlueprintCallable, Category = "VehiclePathTool")
	virtual void CopyTargetNodeWaitTime(AFGTargetPoint* targetPoint);

	UFUNCTION(BlueprintCallable, Category = "VehiclePathTool")
	virtual void PasteTargetNodeSpeed(AFGTargetPoint* targetPoint);

	UFUNCTION(BlueprintCallable, Category = "VehiclePathTool")
	virtual void PasteTargetNodeWaitTime(AFGTargetPoint* targetPoint);

	UFUNCTION(BlueprintCallable, Category = "VehiclePathTool")
	virtual void EnableAIDriver(AFGWheeledVehicle* vehicle);

	UFUNCTION(BlueprintCallable, Category = "VehiclePathTool")
	virtual void DisableAIDriver(AFGWheeledVehicle* vehicle);

	UPROPERTY(BlueprintReadWrite)
	UStaticMesh* splinePathMesh;

	UPROPERTY(BlueprintReadWrite)
	TArray<UMaterialInterface*> splinePathMaterials;

	UPROPERTY(BlueprintReadWrite)
	UPointLightComponent* indicatorLight;

protected:
	UPROPERTY(BlueprintReadWrite)
		EVPT_Mode currentMode;

	UPROPERTY(BlueprintReadWrite)
		AFGDrivingTargetList* targetDrivingTargetList;

	UPROPERTY(BlueprintReadWrite)
		AFGTargetPoint* targetTargetPoint;

	UPROPERTY(BlueprintReadWrite)
		AFGWheeledVehicle* targetVehicle;

	UPROPERTY(BlueprintReadWrite)
		int copiedNodeSpeed;

	UPROPERTY(BlueprintReadWrite)
		float copiedNodeWaitTime;

private:
	TArray<USplineMeshComponent*> pathMeshPool;

	AActor* hitActor;

	int currentMaterialIndex = 0;
	UMaterialInterface* GetNextMaterial();
};

