

#pragma once

#include "CoreMinimal.h"
#include "Hologram/FGConveyorAttachmentHologram.h"
#include "LBBuildableBalancer3.h"
#include "LBDynamicBalancerHologram.generated.h"

UENUM()
	 enum class EDynamicBalancerHologramBuildStep : uint8
 {
	 DBHBS_PlacementAndRotation,
	 DBHBS_AdjustLength
 };

UCLASS()
class LOADBALANCERS_API ALBDynamicBalancerHologram : public AFGConveyorAttachmentHologram
{
	GENERATED_BODY()

public:
		virtual void BeginPlay() override;
		virtual bool DoMultiStepPlacement(bool isInputFromARelease) override;
		virtual void SetHologramLocationAndRotation(const FHitResult& hitResult) override;
		virtual void ScrollRotate(int32 delta, int32 step) override;
		virtual int32 GetRotationStep() const override;
		virtual void ConfigureComponents(AFGBuildable* inBuildable) const override;
		virtual void ConfigureActor(AFGBuildable* inBuildable) const override;

		EDynamicBalancerHologramBuildStep mBuildStep = EDynamicBalancerHologramBuildStep::DBHBS_PlacementAndRotation;

		UPROPERTY()
		int additionalChunks;

		TArray<UFGColoredInstanceMeshProxy*> AddedComponents;

		UPROPERTY(EditDefaultsOnly)
		UStaticMesh* savedStaticMesh;

		//UPROPERTY(EditDefaultsOnly)
		//	TSubclassOf<UFGRecipe> lb2Recipe;
		//UPROPERTY(EditDefaultsOnly)
		//	TSubclassOf<UFGRecipe> lb3Recipe;
		//UPROPERTY(EditDefaultsOnly)
		//	TSubclassOf<UFGRecipe> lb4Recipe;
		//UPROPERTY(EditDefaultsOnly)
		//	TSubclassOf<UFGRecipe> lb5Recipe;
		//UPROPERTY(EditDefaultsOnly)
		//	TSubclassOf<UFGRecipe> lb6Recipe;
		//UPROPERTY(EditDefaultsOnly)
		//	TSubclassOf<UFGRecipe> lb7Recipe;
		//UPROPERTY(EditDefaultsOnly)
		//	TSubclassOf<UFGRecipe> lb8Recipe;
		//UPROPERTY(EditDefaultsOnly)
		//	TSubclassOf<UFGRecipe> lb9Recipe;
		//UPROPERTY(EditDefaultsOnly)
		//	TSubclassOf<UFGRecipe> lb10Recipe;

		UPROPERTY(EditDefaultsOnly)
			TMap<int, TSubclassOf<ALBBuildableBalancer3>> BuildableMap;

		bool flipper;

};
