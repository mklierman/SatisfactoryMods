#pragma once
#include "Equipment/FGEquipment.h"
#include "CoreMinimal.h"
#include "FGColoredInstanceMeshProxy.h"
#include "LBBuild_ModularLoadBalancer.h"
#include "DT_EquipDigbyTool.generated.h"

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

	UPROPERTY(BlueprintReadWrite)
	ALBBuild_ModularLoadBalancer* firstGroupLeader;

	UPROPERTY(BlueprintReadWrite)
	ALBBuild_ModularLoadBalancer* secondGroupLeader;

	UPROPERTY(BlueprintReadWrite)
		TArray< ALBBuild_ModularLoadBalancer*> splitBalancers;

	UPROPERTY(BlueprintReadWrite)
		TArray< ALBBuild_ModularLoadBalancer*> balancersToSplit;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void MergeGroups();

	UFUNCTION(BlueprintCallable, Category = "Equipment")
		void SplitGroups();

	UFUNCTION(BlueprintCallable, Category = "Equipment")
		EToolMode GetCurrentToolMode();

	UFUNCTION(BlueprintCallable, Category = "Equipment")
		void SetCurrentToolMode(EToolMode newMode);
};

