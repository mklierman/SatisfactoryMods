// ILikeBanas

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hologram/FGFoundationHologram.h"

#include "Hologram_PrimaryFoundation.generated.h"

UCLASS()
class UNIFIEDGRID_API AHologram_PrimaryFoundation : public AFGFoundationHologram
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHologram_PrimaryFoundation();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void CheckValidPlacement() override;

	virtual bool TryUpgrade(const FHitResult& hitresult) override;

private:
	bool alreadyBuilt;

	virtual void Scroll(int32 delta) override;
	virtual void ScrollRotate(int32 delta, int32 step) override;
};
