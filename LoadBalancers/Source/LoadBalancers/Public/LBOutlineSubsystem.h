

#pragma once

#include "CoreMinimal.h"
#include "FGGameUserSettings.h"
#include "Subsystem/ModSubsystem.h"
#include "LBBuild_ModularLoadBalancer.h"
#include "LBOutlineActor.h"
#include "LBOutlineSubsystem.generated.h"

/**
Subsystem To create and replicate Outlines
*/
UCLASS()
class LOADBALANCERS_API ALBOutlineSubsystem : public AModSubsystem
{
	GENERATED_BODY()
	public:
	
	UFUNCTION( BlueprintPure, Category = "Subsystem", DisplayName = "GetALBOutlineSubsystem", meta = ( DefaultToSelf = "WorldContext" ) )
	static ALBOutlineSubsystem* Get(UObject* worldContext);

	virtual void BeginPlay() override;

	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_CreateOutlineForActor(AActor* Actor);

	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_ClearOutlines();

	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_SetOutlineColor(FLinearColor Color);

	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_ResetOutlineColor();

	/** Create a new Outline Actor based on the target Actor */
	UFUNCTION(BlueprintCallable)
	void CreateOutline(AActor* Actor, bool Multicast = false);
	
	/** Remove all Outline Actors */
	UFUNCTION(BlueprintCallable)
	void ClearOutlines(bool Multicast = false);
	
	/** Overwrite the DismantleColor */
	UFUNCTION(BlueprintCallable)
	void SetOutlineColor(FLinearColor Color, bool Multicast = false);
	
	/** Reset the DismantleColor to mCachedDismantleColor */
	UFUNCTION(BlueprintCallable)
	void ResetOutlineColor(bool Multicast = false);

	UPROPERTY(Transient)
	TMap<AActor*, ALBOutlineActor*> mOutlineMap;

	UPROPERTY()
	FVector mCachedDismantleColor;

	UPROPERTY(Transient)
	UFGGameUserSettings* mGameUserSettings;
	
	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* mHoloMaterial;
};
