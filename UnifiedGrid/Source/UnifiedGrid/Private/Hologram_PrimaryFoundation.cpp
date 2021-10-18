// ILikeBanas


#include "Hologram_PrimaryFoundation.h"
#include "UnifiedGridModule.h"
#include "Building_PrimaryFoundation.h"
#include "FGConstructDisqualifier.h"

// Sets default values
AHologram_PrimaryFoundation::AHologram_PrimaryFoundation()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	SetActorTickEnabled(true);
}

// Called when the game starts or when spawned
void AHologram_PrimaryFoundation::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuilding_PrimaryFoundation::StaticClass(), FoundActors);

	if (FoundActors.Num() > 0)
	{
		alreadyBuilt = true;
	}
	else
	{
		alreadyBuilt = false;
	}
}

// Called every frame
void AHologram_PrimaryFoundation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHologram_PrimaryFoundation::CheckValidPlacement()
{
	if (alreadyBuilt)
	{
		this->AddConstructDisqualifier(UFGCDUniqueBuilding::StaticClass());
	}
	AFGFoundationHologram::CheckValidPlacement();
}

bool AHologram_PrimaryFoundation::TryUpgrade(const FHitResult& hitresult)
{
	return AFGFoundationHologram::TryUpgrade(hitresult);
}

void AHologram_PrimaryFoundation::Scroll(int32 delta)
{
}

void AHologram_PrimaryFoundation::ScrollRotate(int32 delta, int32 step)
{
}


