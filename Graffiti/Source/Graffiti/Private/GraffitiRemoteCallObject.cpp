#include "GraffitiRemoteCallObject.h"

#include "FGPlayerController.h"
#include "FGBlueprintSubsystem.h"
#include "FGBuildableGraffitiContainer.h"
#include "Buildables/FGBuildableBlueprintDesigner.h"
#include "GraffitiConstants.h"
#include "GraffitiIconUtils.h"
#include "GraffitiPlacementMath.h"
#include "GraffitiSubsystem.h"
#include "Net/UnrealNetwork.h"

void UGraffitiRemoteCallObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGraffitiRemoteCallObject, DummyReplicatedField);
}

UGraffitiRemoteCallObject* UGraffitiRemoteCallObject::Get(UObject* WorldContext)
{
	if (!WorldContext || !WorldContext->GetWorld())
	{
		return nullptr;
	}

	if (AFGPlayerController* Controller = Cast<AFGPlayerController>(WorldContext->GetWorld()->GetFirstPlayerController()))
	{
		return Controller->GetRemoteCallObjectOfClass<UGraffitiRemoteCallObject>();
	}
	return nullptr;
}

bool UGraffitiRemoteCallObject::ServerSpray_Validate(
	const FPersistentGlobalIconId& InIconId,
	float InSize,
	float RollDegrees,
	float MaximumDistance,
	bool,
	bool,
	float GridSize)
{
	return InIconId.IconID >= 0 &&
		FMath::IsFinite(InSize) && InSize >= Graffiti::MinSize && InSize <= Graffiti::MaxSize &&
		FMath::IsFinite(RollDegrees) &&
		FMath::IsFinite(MaximumDistance) && MaximumDistance >= Graffiti::MinDistance && MaximumDistance <= Graffiti::MaxDistance &&
		FMath::IsFinite(GridSize) && GridSize >= Graffiti::MinGridSize && GridSize <= Graffiti::MaxGridSize;
}

void UGraffitiRemoteCallObject::ServerSpray_Implementation(
	const FPersistentGlobalIconId& InIconId,
	float InSize,
	float RollDegrees,
	float MaximumDistance,
	bool bMirrorHorizontal,
	bool bSnapToGrid,
	float GridSize)
{
	if (!Graffiti::ResolveIconTexture(this, InIconId))
	{
		return;
	}

	FHitResult Hit;
	if (!TraceFromOwner(MaximumDistance, Hit))
	{
		return;
	}

	const FQuat Rotation = GraffitiPlacementMath::MakeDecalRotation(Hit.ImpactNormal, RollDegrees);
	const FVector PlacementPoint = bSnapToGrid
		? GraffitiPlacementMath::SnapPointToWorldGrid(Hit.ImpactPoint, Hit.ImpactNormal, GridSize)
		: Hit.ImpactPoint;
	const FVector Location = PlacementPoint + Hit.ImpactNormal * Graffiti::SurfaceOffset;

	const FTransform Transform(Rotation, Location);
	if (AFGBlueprintSubsystem* BlueprintSubsystem = AFGBlueprintSubsystem::Get(GetWorld()))
	{
		if (AFGBuildableBlueprintDesigner* Designer = BlueprintSubsystem->IsLocationInsideABlueprintDesigner(Location))
		{
			AFGBuildableGraffitiContainer* Container = nullptr;
			for (AFGBuildable* Buildable : Designer->GetBuildablesInBlueprintDesigner())
			{
				if (AFGBuildableGraffitiContainer* ExistingContainer = Cast<AFGBuildableGraffitiContainer>(Buildable))
				{
					Container = ExistingContainer;
					break;
				}
			}

			if (!Container)
			{
				Container = GetWorld()->SpawnActorDeferred<AFGBuildableGraffitiContainer>(
					AFGBuildableGraffitiContainer::StaticClass(),
					Designer->GetActorTransform(),
					nullptr,
					nullptr,
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
				if (Container)
				{
					Container->SetInsideBlueprintDesigner(Designer);
					Container->FinishSpawning(Designer->GetActorTransform());
					Designer->OnBuildableConstructedInsideDesigner(Container);
				}
			}

			if (Container)
			{
				if (Container->AddGraffiti(Transform, InIconId, InSize, bMirrorHorizontal).IsValid())
				{
					return;
				}
			}
		}
	}

	if (AGraffitiSubsystem* Subsystem = AGraffitiSubsystem::Get(this))
	{
		Subsystem->AddGraffiti(Transform, InIconId, InSize, bMirrorHorizontal);
	}
}

bool UGraffitiRemoteCallObject::ServerErase_Validate(float MaximumDistance)
{
	return FMath::IsFinite(MaximumDistance) && MaximumDistance >= 100.0f && MaximumDistance <= 3000.0f;
}

void UGraffitiRemoteCallObject::ServerErase_Implementation(float MaximumDistance)
{
	FHitResult Hit;
	if (TraceFromOwner(MaximumDistance, Hit))
	{
		if (AGraffitiSubsystem* Subsystem = AGraffitiSubsystem::Get(this))
		{
			Subsystem->RemoveClosestGraffiti(Hit.ImpactPoint);
		}
	}
}

bool UGraffitiRemoteCallObject::TraceFromOwner(float MaximumDistance, FHitResult& OutHit) const
{
	AFGPlayerController* Controller = GetOwnerPlayerController();
	if (!Controller || !GetWorld())
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GraffitiSpray), true, Controller->GetPawn());
	return GetWorld()->LineTraceSingleByChannel(OutHit, ViewLocation, ViewLocation + ViewRotation.Vector() * MaximumDistance, ECC_Visibility, QueryParams);
}
