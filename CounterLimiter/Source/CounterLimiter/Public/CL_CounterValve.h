#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CL_CounterValve.generated.h"

UCLASS()
class COUNTERLIMITER_API ACL_CounterValve : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACL_CounterValve();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
