// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbstractInstanceInterface.h"
#include "FGBlueprintFunctionLibrary.h"
#include "FGColoredInstanceMeshProxy.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ActorUtilities.generated.h"

#pragma optimize("", off)
/**
 * 
 */
UCLASS()
class REARCHITECT_API UActorUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	static void ShowOutlineMultiForActors( AActor* ActorToAttachProxyComponentsTo, const TArray<AActor*>& InActors, TMap<UStaticMesh*, UInstancedStaticMeshComponent*>& OutProxyOutlineMeshes, EOutlineColor OutlineColor )
	{
		const auto AddProxyOutlineMeshInstance = [&]( UStaticMesh* InStaticMesh, const FTransform& InInstanceTransform )
		{
			// Create a new instanced mesh if we do not have one yet for that mesh type
			if ( !OutProxyOutlineMeshes.Contains( InStaticMesh ) )
			{
				UInstancedStaticMeshComponent* OutlineProxyMeshComponent = NewObject<UInstancedStaticMeshComponent>( ActorToAttachProxyComponentsTo );

				OutlineProxyMeshComponent->SetupAttachment( ActorToAttachProxyComponentsTo->GetRootComponent() );
				OutlineProxyMeshComponent->SetStaticMesh( InStaticMesh );
				OutlineProxyMeshComponent->SetCollisionEnabled( ECollisionEnabled::NoCollision );

				// Disable shadow, disable LODs and disable Nanite for meshes that have it enabled
				OutlineProxyMeshComponent->SetCastShadow( false );
				OutlineProxyMeshComponent->SetForcedLodModel( 0 );
				OutlineProxyMeshComponent->bDisallowNanite = true;

				// Do not render the component in the main or depth pass, but render it in custom depth pass to show the outline
				OutlineProxyMeshComponent->SetRenderInDepthPass( false );
				OutlineProxyMeshComponent->SetRenderInMainPass( false );
				UFGBlueprintFunctionLibrary::ShowOutline( OutlineProxyMeshComponent, OutlineColor );

				OutlineProxyMeshComponent->RegisterComponent();
				OutProxyOutlineMeshes.Add( InStaticMesh, OutlineProxyMeshComponent );
			}

			// Add instance to the existing instanced mesh
			OutProxyOutlineMeshes.FindChecked( InStaticMesh )->AddInstance( InInstanceTransform, true );
		};
	
		for ( AActor* Actor : InActors )
		{
			// Handle normal non-instanced static meshes and legacy instanced meshes
			for ( UStaticMeshComponent* StaticMeshComponent : TInlineComponentArray<UStaticMeshComponent*>( Actor ) )
			{
				// Handle legacy instanced mesh by creating a proxy component for it if it is indeed instanced
				const UFGColoredInstanceMeshProxy* InstancedMeshProxy = Cast<UFGColoredInstanceMeshProxy>( StaticMeshComponent );
				if ( InstancedMeshProxy && InstancedMeshProxy->mInstanceHandle.IsInstanced() )
				{
					AddProxyOutlineMeshInstance( InstancedMeshProxy->GetStaticMesh(), InstancedMeshProxy->GetComponentTransform() );
					continue;
				}

				// Handle instanced factory legs (note that vanilla currently does not do this correctly)
				//const UFGFactoryLegInstanceMeshProxy* FactoryLegMeshProxy = Cast<UFGFactoryLegInstanceMeshProxy>( StaticMeshComponent );
				//if ( FactoryLegMeshProxy && FactoryLegMeshProxy->mInstanceHandle.IsInstanced() )
				//{
				//	AddProxyOutlineMeshInstance( FactoryLegMeshProxy->GetStaticMesh(), FactoryLegMeshProxy->GetComponentTransform() );
				//	continue;
				//}

				// Handle non-instanced mesh components by just setting custom data on them
				UFGBlueprintFunctionLibrary::ShowOutline( StaticMeshComponent, OutlineColor );
			}

			// Handle lightweight instances (new system).
			// Cast will only succeed if the interface is implemented in native code, which means we can use GetLightweightInstanceHandles (which is only implemented in C++)
			if ( const IAbstractInstanceInterface* AbstractInstanceInterface = Cast<IAbstractInstanceInterface>( Actor ) )
			{
				for ( const FInstanceHandle* InstanceHandle : AbstractInstanceInterface->GetLightweightInstanceHandles().HandleArray )
				{
					if ( InstanceHandle->IsInstanced() && InstanceHandle->IsValid() )
					{
						// Retrieve instance transform in world space
						FTransform InstanceTransform;
						InstanceHandle->GetInstanceComponent()->GetInstanceTransform( InstanceHandle->GetHandleID(), InstanceTransform, true );

						AddProxyOutlineMeshInstance( InstanceHandle->GetInstanceComponent()->GetStaticMesh(), InstanceTransform );
					}
				}
			}
			// GetActorLightweightInstanceData does not account for dynamically spawned instances, or for instance visibility, but is BP friendly, so use it as a fallback
			else if ( Actor->Implements<UAbstractInstanceInterface>() )
			{
				const FTransform ActorTransform = IAbstractInstanceInterface::Execute_GetLightweightInstanceActorTransform( Actor );

				for ( const FInstanceData& InstanceData : IAbstractInstanceInterface::Execute_GetActorLightweightInstanceData( Actor ) )
				{
					if ( InstanceData.StaticMesh != nullptr )
					{
						// World transform is the actor transform with the relative instance transform applied on top
						AddProxyOutlineMeshInstance( InstanceData.StaticMesh, InstanceData.RelativeTransform * ActorTransform );
					}
				}
			}
		}
	}

	static void ShowOutlineMultiForActors( AActor* ActorToAttachProxyComponentsTo, const TMap<AActor*, EOutlineColor>& InActors, TMap<UStaticMesh*, UInstancedStaticMeshComponent*>& OutProxyOutlineMeshes)
	{
		const auto AddProxyOutlineMeshInstance = [&]( UStaticMesh* InStaticMesh, const FTransform& InInstanceTransform, EOutlineColor Color )
		{
			// Create a new instanced mesh if we do not have one yet for that mesh type
			if ( !OutProxyOutlineMeshes.Contains( InStaticMesh ) )
			{
				UInstancedStaticMeshComponent* OutlineProxyMeshComponent = NewObject<UInstancedStaticMeshComponent>( ActorToAttachProxyComponentsTo );

				OutlineProxyMeshComponent->SetupAttachment( ActorToAttachProxyComponentsTo->GetRootComponent() );
				OutlineProxyMeshComponent->SetStaticMesh( InStaticMesh );
				OutlineProxyMeshComponent->SetCollisionEnabled( ECollisionEnabled::NoCollision );

				// Disable shadow, disable LODs and disable Nanite for meshes that have it enabled
				OutlineProxyMeshComponent->SetCastShadow( false );
				OutlineProxyMeshComponent->SetForcedLodModel( 0 );
				OutlineProxyMeshComponent->bDisallowNanite = true;

				// Do not render the component in the main or depth pass, but render it in custom depth pass to show the outline
				OutlineProxyMeshComponent->SetRenderInDepthPass( false );
				OutlineProxyMeshComponent->SetRenderInMainPass( false );
				UFGBlueprintFunctionLibrary::ShowOutline( OutlineProxyMeshComponent, Color );

				OutlineProxyMeshComponent->RegisterComponent();
				OutProxyOutlineMeshes.Add( InStaticMesh, OutlineProxyMeshComponent );
			}

			// Add instance to the existing instanced mesh
			OutProxyOutlineMeshes.FindChecked( InStaticMesh )->AddInstance( InInstanceTransform, true );
		};
	
		for ( auto Data : InActors )
		{
			auto Actor = Data.Key;
			auto Color = Data.Value;
			// Handle normal non-instanced static meshes and legacy instanced meshes
			for ( UStaticMeshComponent* StaticMeshComponent : TInlineComponentArray<UStaticMeshComponent*>( Actor ) )
			{
				// Handle legacy instanced mesh by creating a proxy component for it if it is indeed instanced
				const UFGColoredInstanceMeshProxy* InstancedMeshProxy = Cast<UFGColoredInstanceMeshProxy>( StaticMeshComponent );
				if ( InstancedMeshProxy && InstancedMeshProxy->mInstanceHandle.IsInstanced() )
				{
					AddProxyOutlineMeshInstance( InstancedMeshProxy->GetStaticMesh(), InstancedMeshProxy->GetComponentTransform(), Color );
					continue;
				}

				// Handle instanced factory legs (note that vanilla currently does not do this correctly)
				//const UFGFactoryLegInstanceMeshProxy* FactoryLegMeshProxy = Cast<UFGFactoryLegInstanceMeshProxy>( StaticMeshComponent );
				//if ( FactoryLegMeshProxy && FactoryLegMeshProxy->mInstanceHandle.IsInstanced() )
				//{
				//	AddProxyOutlineMeshInstance( FactoryLegMeshProxy->GetStaticMesh(), FactoryLegMeshProxy->GetComponentTransform(), Color );
				//	continue;
				//}

				// Handle non-instanced mesh components by just setting custom data on them
				UFGBlueprintFunctionLibrary::ShowOutline( StaticMeshComponent, Color );
			}

			// Handle lightweight instances (new system).
			// Cast will only succeed if the interface is implemented in native code, which means we can use GetLightweightInstanceHandles (which is only implemented in C++)
			if ( const IAbstractInstanceInterface* AbstractInstanceInterface = Cast<IAbstractInstanceInterface>( Actor ) )
			{
				for ( const FInstanceHandle* InstanceHandle : AbstractInstanceInterface->GetLightweightInstanceHandles().HandleArray )
				{
					if ( InstanceHandle->IsInstanced() && InstanceHandle->IsValid() )
					{
						// Retrieve instance transform in world space
						FTransform InstanceTransform;
						InstanceHandle->GetInstanceComponent()->GetInstanceTransform( InstanceHandle->GetHandleID(), InstanceTransform, true );

						AddProxyOutlineMeshInstance( InstanceHandle->GetInstanceComponent()->GetStaticMesh(), InstanceTransform, Color );
					}
				}
			}
			// GetActorLightweightInstanceData does not account for dynamically spawned instances, or for instance visibility, but is BP friendly, so use it as a fallback
			else if ( Actor->Implements<UAbstractInstanceInterface>() )
			{
				const FTransform ActorTransform = IAbstractInstanceInterface::Execute_GetLightweightInstanceActorTransform( Actor );

				for ( const FInstanceData& InstanceData : IAbstractInstanceInterface::Execute_GetActorLightweightInstanceData( Actor ) )
				{
					if ( InstanceData.StaticMesh != nullptr )
					{
						// World transform is the actor transform with the relative instance transform applied on top
						AddProxyOutlineMeshInstance( InstanceData.StaticMesh, InstanceData.RelativeTransform * ActorTransform, Color );
					}
				}
			}
		}
	}

	static void HideOutlineMultiForActors( const TArray<AActor*>& InActors, TMap<UStaticMesh*, UInstancedStaticMeshComponent*>& ProxyOutlineMeshes )
	{
		for ( AActor* Actor : InActors )
		{
			// Handle normal non-instanced static meshes
			for ( UStaticMeshComponent* StaticMeshComponent : TInlineComponentArray<UStaticMeshComponent*>( Actor ) )
			{
				// Do not attempt to un-hide instanced colored meshes
				const UFGColoredInstanceMeshProxy* InstancedMeshProxy = Cast<UFGColoredInstanceMeshProxy>( StaticMeshComponent );
				if ( InstancedMeshProxy && InstancedMeshProxy->mInstanceHandle.IsInstanced() )
				{
					continue;
				}

				// Do not attempt to un-hide instanced factory legs
				//const UFGFactoryLegInstanceMeshProxy* FactoryLegMeshProxy = Cast<UFGFactoryLegInstanceMeshProxy>( StaticMeshComponent );
				//if ( FactoryLegMeshProxy && FactoryLegMeshProxy->mInstanceHandle.IsInstanced() )
				//{
				//	continue;
				//}
				// Reset the custom data on the mesh
				UFGBlueprintFunctionLibrary::HideOutline( StaticMeshComponent );
			}
		}

		// Destroy all proxy outline components
		for ( const TPair<UStaticMesh*, UInstancedStaticMeshComponent*>& Pair : ProxyOutlineMeshes )
		{
			Pair.Value->DestroyComponent();
		}
		ProxyOutlineMeshes.Empty();
	}

	UFUNCTION()
	static void RefreshInstanceHandle(UFGColoredInstanceMeshProxy* MeshProxy)
	{
		auto& Handle = MeshProxy->mInstanceHandle;
				
		if(Handle.GetHandleID() >= 0) MeshProxy->mInstanceManager->RemoveInstance(MeshProxy->mInstanceHandle);
		if(Handle.GetHandleID() < 0) MeshProxy->InstantiateInternal();
	}
};
#pragma optimize("", on)
