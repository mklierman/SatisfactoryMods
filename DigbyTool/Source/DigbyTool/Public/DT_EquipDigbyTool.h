#pragma once
#include "Equipment/FGEquipment.h"
#include "CoreMinimal.h"
#include "DT_EquipDigbyTool.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(DigbyTool_Log, Display, All);

UENUM(BlueprintType)
enum class EToolMode : uint8
{
	Merge = 0 UMETA(DisplayName = "Merge"),
	Split = 1 UMETA(DisplayName = "Split"),
};


UENUM(BlueprintType)
enum class EToolError : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	MergeOverflows = 1 UMETA(DisplayName = "Merge Overflows"),
	SplitOverflows = 2 UMETA(DisplayName = "Split Overflows"),
};

UCLASS()
class DIGBYTOOL_API ADT_EquipDigbyTool : public AFGEquipment
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	EToolMode currentToolMode;

	UPROPERTY(BlueprintReadWrite)
	EToolError currentToolError;

	UPROPERTY(EditDefaultsOnly, Category = "Holo")
	UMaterialInterface* firstGroupHoloMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Holo")
	UMaterialInterface* secondGroupHoloMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Holo")
	UMaterialInterface* mainMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Holo")
	UMaterial* secondaryMaterial;
};

