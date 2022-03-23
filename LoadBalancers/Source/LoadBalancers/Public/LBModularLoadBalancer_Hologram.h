

#pragma once

#include "FactoryGame.h"
#include "CoreMinimal.h"
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
	virtual void BeginPlay() override;
	virtual bool TrySnapToActor(const FHitResult& hitResult) override;

	void Server_SnapStuff(bool SnapResult);

	void UnHighlightAll();

	void HighlightBalancer(ALBBuild_ModularLoadBalancer* Balancer);

	void HighlightAll(TArray<ALBBuild_ModularLoadBalancer*> actorsToOutline);

	void SetMeshInstanced(UMeshComponent* MeshComp, bool Instanced);

	void UnHighlightBalancer(ALBBuild_ModularLoadBalancer* Balancer);

	UPROPERTY()
	FVector cachedDismantleColor = FVector(-1.0, -1.0, -1.0);


	UPROPERTY(BlueprintReadWrite, SaveGame, Replicated)
	ALBBuild_ModularLoadBalancer* LastSnapped;

	UPROPERTY(BlueprintReadWrite, SaveGame, Replicated)
	TArray<ALBBuild_ModularLoadBalancer*> HologrammedBalancers;

};
