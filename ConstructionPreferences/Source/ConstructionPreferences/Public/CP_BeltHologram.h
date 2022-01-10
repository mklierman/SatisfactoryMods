

#pragma once

#include "CoreMinimal.h"
#include "Hologram/FGConveyorBeltHologram.h"
#include "CP_BeltHologram.generated.h"

/**
 * 
 */
UCLASS()
class CONSTRUCTIONPREFERENCES_API ACP_BeltHologram : public AFGConveyorBeltHologram
{
	GENERATED_BODY()
public:
	virtual void SpawnChildren(AActor* hologramOwner, FVector spawnLocation, APawn* hologramInstigator) override;
private:
	
};
