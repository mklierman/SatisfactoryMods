// ILikeBanas

#pragma once

#include "CoreMinimal.h"

#include "Buildables/FGBuildableFoundation.h"
#include "GameFramework/Actor.h"
#include "Build_OutpostFoundation.generated.h"

UCLASS()
class UNIFIEDGRID_API ABuild_OutpostFoundation : public AFGBuildableFoundation
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABuild_OutpostFoundation();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
