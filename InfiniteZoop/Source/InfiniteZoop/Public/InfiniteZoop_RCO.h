

#pragma once

#include "CoreMinimal.h"
#include "FGPlayerController.h"
#include "FGRemoteCallObject.h"
#include "Subsystem/SubsystemActorManager.h"
#include "InfiniteZoopSubsystem.h"
#include <Hologram/FGFactoryBuildingHologram.h>
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

    UFUNCTION(BlueprintCallable, Category = "Networking", meta = (WorldContext = "WorldContext"))
    static UInfiniteZoop_RCO* GetFFDefaultRCO(UObject* WorldContext, AFGPlayerController* Controller) { return Get(WorldContext, Controller); }
    static UInfiniteZoop_RCO* Get(UObject* WorldContext, AFGPlayerController* Controller)
    {
        if (WorldContext)
            if (Controller)
                if (UInfiniteZoop_RCO* RCO = Controller->GetRemoteCallObjectOfClass<UInfiniteZoop_RCO>())
                    return RCO;
        return nullptr;
    };

    //UFUNCTION(Reliable, Server)
    //void GetZoopAmount(UWorld* world, const int& OutCurrentZoopAmount);

    //UFUNCTION(Reliable, Server)
    //void GetZoopCorners(UWorld* world, bool const& OutZoopCorners);

    UFUNCTION(Reliable, Server)
    void SetHologramMaxZoop(AFGFactoryBuildingHologram* hologram, int newMaxZoop);
private:

    /** Needed for RCO to work */
    UPROPERTY(Replicated)
    bool DummyReplicatedField = true;
};
