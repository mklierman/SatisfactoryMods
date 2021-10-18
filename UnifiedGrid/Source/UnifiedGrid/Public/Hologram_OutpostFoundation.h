// ILikeBanas

#pragma once

#include "CoreMinimal.h"
#include "Hologram/FGFoundationHologram.h"
#include "GameFramework/Actor.h"
#include "Hologram_OutpostFoundation.generated.h"

UCLASS()
class UNIFIEDGRID_API AHologram_OutpostFoundation : public AFGFoundationHologram
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHologram_OutpostFoundation();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual bool IsValidHitResult(const FHitResult& hit) const override;
	virtual bool TryUpgrade(const FHitResult& hitResult) override;
};
