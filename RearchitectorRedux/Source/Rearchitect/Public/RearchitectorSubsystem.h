// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorUtilities.h"
#include "Components/BillboardComponent.h"
#include "Subsystem/ModSubsystem.h"
#include "RearchitectorSubsystem.generated.h"

UCLASS()
class AOutlineProxyActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UStaticMeshComponent* MassSelectMesh;
};

UCLASS()
class REARCHITECT_API ARearchitectorSubsystem : public AModSubsystem
{
	GENERATED_BODY()

	ARearchitectorSubsystem()
	{
		ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnClient;
		SetRootComponent(CreateDefaultSubobject<USceneComponent>("Root"));
#if WITH_EDITORONLY_DATA
		GetSpriteComponent()->DestroyComponent();
#endif
	}

	virtual void BeginPlay() override
	{
		Super::BeginPlay();
		Self = this;
		Proxy = GetWorld()->SpawnActor<AOutlineProxyActor>();

		UStaticMeshComponent* OutlineProxyMeshComponent = NewObject<UStaticMeshComponent>( Proxy );

		OutlineProxyMeshComponent->SetupAttachment( Proxy->GetRootComponent() );
		OutlineProxyMeshComponent->SetStaticMesh( GetMassSelectOverlapMesh() );
		OutlineProxyMeshComponent->SetCollisionEnabled( ECollisionEnabled::QueryOnly );
		OutlineProxyMeshComponent->SetCollisionResponseToAllChannels(ECR_Overlap);

		// Disable shadow, disable LODs and disable Nanite for meshes that have it enabled
		OutlineProxyMeshComponent->SetCastShadow( false );
		OutlineProxyMeshComponent->SetForcedLodModel( 0 );
		OutlineProxyMeshComponent->bDisallowNanite = true;

		// Do not render the component in the main or depth pass, but render it in custom depth pass to show the outline
		OutlineProxyMeshComponent->SetRenderInDepthPass( false );
		OutlineProxyMeshComponent->SetRenderInMainPass( false );
		UFGBlueprintFunctionLibrary::ShowOutline( OutlineProxyMeshComponent, EOutlineColor::OC_HOLOGRAM );

		OutlineProxyMeshComponent->RegisterComponent();
		Proxy->AddInstanceComponent(OutlineProxyMeshComponent);
		Proxy->MassSelectMesh = OutlineProxyMeshComponent;
		OriginalMassSelectBounds = OutlineProxyMeshComponent->GetStaticMesh()->GetBounds().BoxExtent;
	}

public:

	static inline ARearchitectorSubsystem* Self;

	void HideOutline(const TArray<AActor*>& OutlinedActors){ UActorUtilities::HideOutlineMultiForActors(OutlinedActors, OutlineProxyData); }
	
	void RefreshOutline(const TArray<AActor*>& InActors, EOutlineColor OutlineColor = EOutlineColor::OC_HOLOGRAM)
	{
		UActorUtilities::HideOutlineMultiForActors(InActors, OutlineProxyData);
		UActorUtilities::ShowOutlineMultiForActors(Proxy, InActors, OutlineProxyData, OutlineColor);
	}

	void RefreshOutline(const TMap<AActor*, EOutlineColor>& InActors)
	{
		TArray<AActor*> Actors;
		InActors.GenerateKeyArray(Actors);
		UActorUtilities::HideOutlineMultiForActors(Actors, OutlineProxyData);
		UActorUtilities::ShowOutlineMultiForActors(Proxy, InActors, OutlineProxyData);
	}

	UFUNCTION(BlueprintCallable)
	void UpdateMassSelectBounds(const FVector& StartPosition, const FVector& CurrentPosition, EOutlineColor OutlineColor)
	{
		const auto Delta = CurrentPosition - StartPosition;
		const auto BoundingBox = Delta.GetAbs() / 2.0;
		const auto Center = (CurrentPosition + StartPosition) / 2.0;

		FTransform Transform(Center);
		Transform.SetScale3D(BoundingBox / OriginalMassSelectBounds);
		Proxy->MassSelectMesh->SetWorldScale3D(BoundingBox / OriginalMassSelectBounds);
		Proxy->MassSelectMesh->SetWorldLocation(Center);
		UFGBlueprintFunctionLibrary::ShowOutline(Proxy->MassSelectMesh, OutlineColor);
	}

	UFUNCTION(BlueprintCallable)
	void DisableMassSelectMesh()
	{
		Proxy->MassSelectMesh->SetWorldLocation(FVector::ZeroVector);
		Proxy->MassSelectMesh->SetWorldScale3D(FVector::ZeroVector);
		UFGBlueprintFunctionLibrary::HideOutline(Proxy->MassSelectMesh);
	}
	
	const TArray<FOverlapInfo>& GetMassSelectOverlaps() const
	{
		return Proxy->MassSelectMesh->GetOverlapInfos();
	}


	UPROPERTY()
	TMap<UStaticMesh*, UInstancedStaticMeshComponent*> OutlineProxyData;

	UPROPERTY()
	AOutlineProxyActor* Proxy;

	UPROPERTY()
	FVector OriginalMassSelectBounds;

protected:

	UFUNCTION(BlueprintImplementableEvent)
	UStaticMesh* GetMassSelectOverlapMesh() const;
};
