

#pragma once

#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "Subsystem/SubsystemActorManager.h"
#include "InfiniteZoopSubsystem.h"
#include "InfiniteZoop_RCO.generated.h"

struct ZoopVars
{
    int ZoopAmount;
    bool ZoopCorners;
};
/**
 *
 */
UCLASS()
class INFINITEZOOP_API UInfiniteZoop_RCO : public UFGRemoteCallObject
{
	GENERATED_BODY()
public:
    UInfiniteZoop_RCO();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    //UFUNCTION(Reliable, Server)
    //void GetZoopAmount(UWorld* world, const int& OutCurrentZoopAmount);

    //UFUNCTION(Reliable, Server)
    //void GetZoopCorners(UWorld* world, bool const& OutZoopCorners);
private:

    /** Needed for RCO to work */
    UPROPERTY(Replicated)
    bool DummyReplicatedField = true;
};
