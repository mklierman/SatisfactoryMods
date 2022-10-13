// ILikeBanas


#include "Hologram_PrimaryFoundation.h"
#include "UnifiedGridModule.h"
#include "Building_PrimaryFoundation.h"
#include "Build_OutpostFoundation.h"
#include "FGConstructDisqualifier.h"
#include "Resources/FGResourceNodeBase.h"
#include "Resources/FGResourceNodeFrackingCore.h"
#include "UnifiedGrid_ConfigStruct.h"

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

AActor* AHologram_PrimaryFoundation::Construct(TArray<AActor*>& out_children, FNetConstructionID constructionID)
{
	auto config = FUnifiedGrid_ConfigStruct::GetActiveConfig();
	
	int alignmentHeight = config.AlignmentHeight;
	int extraHeight = config.AdditionalHeight;
	long double GRID_SIZE = 800.0;
	long double HEIGHT_SIZE = StaticCast<long double>(alignmentHeight);
	long double EXTRA_HEIGHT = StaticCast<long double>(extraHeight);
	long double primaryX = this->GetActorLocation().X;
	long double primaryY = this->GetActorLocation().Y;
	long double primaryZ = this->GetActorLocation().Z;

	UClass* OutpostClass = LoadObject<UClass>(nullptr, TEXT("/UnifiedGrid/Buildings/OutpostFoundation/Build_OutpostFoundation.Build_OutpostFoundation_C"));
	TArray<AActor*> FoundNodes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFGResourceNodeBase::StaticClass(), FoundNodes);
	for (auto node : FoundNodes)
	{
		auto rNode = Cast< AFGResourceNodeBase>(node);
		if (rNode)
		{
			if (rNode->CanPlaceResourceExtractor() && !rNode->IsOccupied())
			{
				auto fracker = Cast<AFGResourceNodeFrackingCore>(rNode);
				if (fracker)
				{
					EXTRA_HEIGHT += 30;
				}

				long double secondaryX = rNode->GetActorLocation().X;
				long double secondaryY = rNode->GetActorLocation().Y;
				long double secondaryZ = rNode->GetActorLocation().Z + EXTRA_HEIGHT;
				long double newX = primaryX - (roundl((primaryX - secondaryX) / GRID_SIZE) * GRID_SIZE);
				long double newY = primaryY - (roundl((primaryY - secondaryY) / GRID_SIZE) * GRID_SIZE);
				long double newZ = (primaryZ - (roundl((primaryZ - secondaryZ) / HEIGHT_SIZE) * HEIGHT_SIZE));
				if (newZ < secondaryZ)
				{
					newZ += HEIGHT_SIZE;
				}
				auto spawnLocation = FVector(newX, newY, newZ);

				FActorSpawnParameters params;
				params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				FTransform newT = FTransform(this->GetActorRotation(), spawnLocation, FVector(1, 1, 1));
				auto newguy = this->GetWorld()->SpawnActor(OutpostClass, &newT, params);
			}
		}
	}
	return Super::Construct(out_children, constructionID);
}

void AHologram_PrimaryFoundation::Scroll(int32 delta)
{
}

void AHologram_PrimaryFoundation::ScrollRotate(int32 delta, int32 step)
{
}


