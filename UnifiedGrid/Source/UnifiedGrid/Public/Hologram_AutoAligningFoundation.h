// ILikeBanas

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hologram/FGFoundationHologram.h"

#include "Hologram_AutoAligningFoundation.generated.h"

UCLASS()
class UNIFIEDGRID_API AHologram_AutoAligningFoundation : public AFGFoundationHologram
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHologram_AutoAligningFoundation();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
