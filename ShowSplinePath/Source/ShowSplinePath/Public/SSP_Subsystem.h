#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "WheeledVehicles/FGTargetPointLinkedList.h"
#include "WheeledVehicles/FGTargetPoint.h"
#include "WheeledVehicles/FGWheeledVehicle.h"
#include "Components/SplineMeshComponent.h"
#include "InstancedSplineMeshComponent.h"
#include "FGManta.h"

#include "SSP_Subsystem.generated.h"

class USSP_RemoteCallObject;


UCLASS()
class SHOWSPLINEPATH_API ASSP_Subsystem : public AModSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()

public:
	ASSP_Subsystem();

	bool GetHasAuthority();

	virtual bool ShouldSave_Implementation() const override;

	class USSP_RemoteCallObject* GetRCO();

	UFUNCTION(BlueprintCallable)
	void ShowAllMantaPaths(AActor* actor);
	
	UFUNCTION(BlueprintCallable)
	void HideAllMantaPaths(AActor* actor);

	UPROPERTY(BlueprintReadWrite)
	UStaticMesh* splinePathMesh;

	UPROPERTY(BlueprintReadWrite)
	UStaticMesh* mantaPathMesh;


	UPROPERTY(BlueprintReadWrite)
	TMap<AFGManta*, UInstancedSplineMeshComponent*> MantaMeshComponents;

private:
	bool bIsUpdatingPathVisibility = false;

};
