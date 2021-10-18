// ILikeBanas

#pragma once

#include "CoreMinimal.h"

#include "Buildables/FGBuildableFoundation.h"
#include "GameFramework/Actor.h"
#include "Building_PrimaryFoundation.generated.h"

UCLASS()
class UNIFIEDGRID_API ABuilding_PrimaryFoundation : public AFGBuildableFoundation
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABuilding_PrimaryFoundation();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
