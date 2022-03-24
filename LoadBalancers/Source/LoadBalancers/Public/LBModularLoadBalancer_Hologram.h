

#pragma once

#include "FactoryGame.h"
#include "CoreMinimal.h"
#include "FGConstructDisqualifier.h"
#include "Hologram/FGFactoryHologram.h"
#include "LBBuild_ModularLoadBalancer.h"
#include "Subsystem/ModSubsystem.h"
#include "Subsystem/SubsystemActorManager.h"
#include "LoadBalancers_Subsystem_CPP.h"
#include "FGFactoryConnectionComponent.h"
#include "FGPlayerController.h"
#include "LBModularLoadBalancer_Hologram.generated.h"

/**
 *
 */
//DECLARE_LOG_CATEGORY_EXTERN(LoadBalancersHG_Log, Display, All);
UCLASS()
class LOADBALANCERS_API ALBModularLoadBalancer_Hologram : public AFGFactoryHologram
{
	GENERATED_BODY()
public:

	virtual AActor* Construct(TArray< AActor* >& out_children, FNetConstructionID netConstructionID) override;
	virtual void Destroyed() override;
	virtual bool TrySnapToActor(const FHitResult& hitResult) override;

	void UnHighlightAll();

	void HighlightAll(TArray<ALBBuild_ModularLoadBalancer*> actorsToOutline);

	// Let use configure the load balancer.
	virtual void ConfigureActor(AFGBuildable* inBuildable) const override;

	UPROPERTY()
	FVector cachedDismantleColor = FVector(-1.0, -1.0, -1.0);


	UPROPERTY(BlueprintReadWrite, SaveGame, Replicated)
	ALBBuild_ModularLoadBalancer* LastSnapped;

	UPROPERTY(BlueprintReadWrite, SaveGame, Replicated)
	TArray<ALBBuild_ModularLoadBalancer*> HologrammedBalancers;

	TArray<TWeakObjectPtr<AActor>> mOutlineActors;
	TArray<TWeakObjectPtr<UFGColoredInstanceMeshProxy>> mColoredInstanceMeshProxy;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* mHoloMaterial;
};


UCLASS()
class LOADBALANCERS_API UFGCDHasOverflow : public UFGConstructDisqualifier
{
	GENERATED_BODY()

	UFGCDHasOverflow()
	{
		mDisqfualifyingText = FText::FromString(TEXT("Only one Overflow module can be attached"));
	}
};
