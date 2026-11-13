#include "GraffitiRemoteCallObject.h"

#include "EngineUtils.h"
#include "FGPlayerController.h"
#include "GraffitiConstants.h"
#include "GraffitiDecalActor.h"
#include "GraffitiIconUtils.h"
#include "GraffitiPlacementMath.h"
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
	FLinearColor InTint,
	float InSize,
	float RollDegrees,
	float MaximumDistance,
	bool,
	bool,
	float GridSize)
{
	const bool bFiniteTint = FMath::IsFinite(InTint.R) && FMath::IsFinite(InTint.G) && FMath::IsFinite(InTint.B) && FMath::IsFinite(InTint.A);
	return InIconId.IconID >= 0 &&
		bFiniteTint &&
		FMath::IsFinite(InSize) && InSize >= Graffiti::MinSize && InSize <= Graffiti::MaxSize &&
		FMath::IsFinite(RollDegrees) &&
		FMath::IsFinite(MaximumDistance) && MaximumDistance >= Graffiti::MinDistance && MaximumDistance <= Graffiti::MaxDistance &&
		FMath::IsFinite(GridSize) && GridSize >= Graffiti::MinGridSize && GridSize <= Graffiti::MaxGridSize;
}

void UGraffitiRemoteCallObject::ServerSpray_Implementation(
	const FPersistentGlobalIconId& InIconId,
	FLinearColor InTint,
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

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwnerPlayerController();
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	int32 LayerOrder = 1;
	for (TActorIterator<AGraffitiDecalActor> It(GetWorld()); It; ++It)
	{
		const bool bCanOverlap = GraffitiPlacementMath::CanDecalsOverlap(
			Location,
			Hit.ImpactNormal,
			InSize,
			It->GetActorLocation(),
			It->GetActorForwardVector(),
			It->GetGraffitiSize(),
			Graffiti::LayerNormalAlignment,
			Graffiti::LayerPlaneTolerance,
			Graffiti::SquareRadiusScale);
		if (bCanOverlap && It->GetLayerOrder() < MAX_int32)
		{
			LayerOrder = FMath::Max(LayerOrder, It->GetLayerOrder() + 1);
		}
	}

	const FTransform Transform(Rotation, Location);
	if (AGraffitiDecalActor* Graffiti = GetWorld()->SpawnActor<AGraffitiDecalActor>(AGraffitiDecalActor::StaticClass(), Transform, SpawnParameters))
	{
		Graffiti->InitializeGraffiti(InIconId, InTint.GetClamped(), InSize, bMirrorHorizontal, LayerOrder);
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
		AGraffitiDecalActor* ClosestGraffiti = nullptr;
		float ClosestDistanceSquared = TNumericLimits<float>::Max();
		for (TActorIterator<AGraffitiDecalActor> It(GetWorld()); It; ++It)
		{
			AGraffitiDecalActor* Graffiti = *It;
			const float DistanceSquared = FVector::DistSquared(Graffiti->GetActorLocation(), Hit.ImpactPoint);
			const float EraseRadius = Graffiti->GetGraffitiSize() * 0.6f;
			if (DistanceSquared <= FMath::Square(EraseRadius) && DistanceSquared < ClosestDistanceSquared)
			{
				ClosestGraffiti = Graffiti;
				ClosestDistanceSquared = DistanceSquared;
			}
		}

		if (ClosestGraffiti)
		{
			ClosestGraffiti->Destroy();
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
