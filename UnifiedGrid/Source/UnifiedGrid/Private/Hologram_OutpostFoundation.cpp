// ILikeBanas


#include "Hologram_OutpostFoundation.h"

#include "Build_OutpostFoundation.h"
#include "UnifiedGridModule.h"

// Sets default values
AHologram_OutpostFoundation::AHologram_OutpostFoundation()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	this->mValidHitClasses.Add(AFGBuildableFoundation::StaticClass());
}

// Called when the game starts or when spawned
void AHologram_OutpostFoundation::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHologram_OutpostFoundation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AHologram_OutpostFoundation::IsValidHitResult(const FHitResult& hit) const
{
	// AActor* target = hit.GetActor();
	// UE_LOG(UnifiedGrid_Log, Display, TEXT("IsValidHitResult Actor hit: %s"), *target->GetName());
	// if (target->GetClass()->GetName() == "Build_OutpostFoundation_C")
	// {
	// 	return true;
	// }
	return Super::IsValidHitResult(hit);
}

bool AHologram_OutpostFoundation::TryUpgrade(const FHitResult& hitResult)
{
	AActor* target = hitResult.GetActor();
	UE_LOG(UnifiedGrid_Log, Display, TEXT("TryUpgrade Actor hit: %s"), *target->GetName());
	if (target->IsA(AFGBuildableFoundation::StaticClass()))
	{
		return true;
	}
	return Super::TryUpgrade(hitResult);
}
