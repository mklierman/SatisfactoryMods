

#include "LBModularLoadBalancer_Hologram.h"
#include "LoadBalancersModule.h"
#include "FGColoredInstanceMeshProxy.h"
#include "FGProductionIndicatorInstanceComponent.h"
#include "LB_RCO.h"
#include "FGGameUserSettings.h"
#include <Runtime/Engine/Classes/Components/ProxyInstancedStaticMeshComponent.h>

//DEFINE_LOG_CATEGORY(LoadBalancers_Log);

void ALBModularLoadBalancer_Hologram::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALBModularLoadBalancer_Hologram, LastSnapped);
	DOREPLIFETIME(ALBModularLoadBalancer_Hologram, HologrammedBalancers);
}

AActor* ALBModularLoadBalancer_Hologram::Construct(TArray<AActor*>& out_children, FNetConstructionID netConstructionID)
{
	UnHighlightAll();
	return Super::Construct(out_children, netConstructionID);
}

void ALBModularLoadBalancer_Hologram::Destroyed()
{
	if (cachedDismantleColor.X != -1.0)
	{
		UFGGameUserSettings* settings = UFGGameUserSettings::GetFGGameUserSettings();
		settings->SetDismantleHologramColour(cachedDismantleColor);
		settings->ApplyHologramColoursToCollectionParameterInstance(this->GetWorld());
	}
	UnHighlightAll();
	Super::Destroyed();
}

bool ALBModularLoadBalancer_Hologram::TrySnapToActor(const FHitResult& hitResult)
{
	const bool SnapResult = Super::TrySnapToActor(hitResult);
	if (SnapResult)
	{
		ALBBuild_ModularLoadBalancer* Default = GetDefaultBuildable<ALBBuild_ModularLoadBalancer>();
		if (ALBBuild_ModularLoadBalancer* SnappedBalancer = Cast<ALBBuild_ModularLoadBalancer>(hitResult.GetActor()))
		{
			if(Default->mLoaderType == ELoaderType::Overflow)
				if(SnappedBalancer->HasOverflowLoader())
					AddConstructDisqualifier(UFGCDHasOverflow::StaticClass());
			
			if (LastSnapped && SnappedBalancer != LastSnapped)
			{
				UnHighlightAll();
			}
			
			LastSnapped = SnappedBalancer;

			HighlightAll(SnappedBalancer->GetGroupModules());
		}
	}
	else
	{
		if (LastSnapped)
		{
			UnHighlightAll();
			LastSnapped = nullptr;
		}
	}

	return SnapResult;
}

void ALBModularLoadBalancer_Hologram::UnHighlightAll()
{
	for (TWeakObjectPtr<AActor> Actor : mOutlineActors)
		if(Actor.IsValid())
		{
			Actor.Get()->K2_DestroyActor();
		}

	for (TWeakObjectPtr<UFGColoredInstanceMeshProxy> MeshProxy : mColoredInstanceMeshProxy)
		if(MeshProxy.IsValid())
		{
			MeshProxy->SetHiddenInGame(false);
			if(!MeshProxy->mInstanceHandle.IsInstanced())
				MeshProxy->SetInstanced(true);
		}
	
	mColoredInstanceMeshProxy.Empty();
	mOutlineActors.Empty();
}

void ALBModularLoadBalancer_Hologram::ConfigureActor(AFGBuildable* inBuildable) const
{
	if(ALBBuild_ModularLoadBalancer* Balancer = Cast<ALBBuild_ModularLoadBalancer>(inBuildable))
		if(LastSnapped)
			Balancer->GroupLeader = LastSnapped->GroupLeader;
		else
			Balancer->GroupLeader = Balancer;
	
	Super::ConfigureActor(inBuildable);
}

void ALBModularLoadBalancer_Hologram::HighlightAll(TArray<ALBBuild_ModularLoadBalancer*> actorsToOutline)
{
	if(actorsToOutline.Num() <= 0)
		return;
	
	if (cachedDismantleColor.X == -1.0)
	{
		auto settings = UFGGameUserSettings::GetFGGameUserSettings();
		UE_LOG(LoadBalancers_Log, Display, TEXT("Cached Color: %f, %f, %f"), cachedDismantleColor.X, cachedDismantleColor.Y, cachedDismantleColor.Z);
		cachedDismantleColor = settings->mDismantleHologramColour;
		UE_LOG(LoadBalancers_Log, Display, TEXT("Cached Color: %f, %f, %f"), cachedDismantleColor.X, cachedDismantleColor.Y, cachedDismantleColor.Z);
		settings->SetDismantleHologramColour(FVector(0.5, 0.5, 0.5));
		settings->ApplyHologramColoursToCollectionParameterInstance(this->GetWorld());
	}
	
	if(LastSnapped)
	{
		for (AActor* OutlineActor : TArray<AActor*>(actorsToOutline))
			if(UFGColoredInstanceMeshProxy* Proxy = OutlineActor->FindComponentByClass<UFGColoredInstanceMeshProxy>())
			{
				FTransform Transform = LastSnapped->GetTransform();
				if(AActor* OutlinerActor = GetWorld()->SpawnActorDeferred<AActor>(AActor::StaticClass(), Transform))
					if(UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(OutlinerActor))
					{
						Component->RegisterComponent();
						OutlinerActor->FinishSpawning(Transform, true);

						Component->SetRelativeTransform(Proxy->GetRelativeTransform());
						Component->AttachToComponent(OutlineActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
						Component->SetStaticMesh(Proxy->GetStaticMesh());
						Component->SetRenderCustomDepth(true);
						Component->SetCustomDepthStencilValue(250);

						for(int i = 0; i < Component->GetNumMaterials(); ++i)
							Component->SetMaterial(i, mHoloMaterial);
						
						Proxy->SetHiddenInGame(true);
						if(Proxy->mInstanceHandle.IsInstanced())
							Proxy->SetInstanced(false);

						mColoredInstanceMeshProxy.Add(Proxy);
						
						OutlinerActor->SetReplicates(true);
						OutlinerActor->ForceNetUpdate();
						
						mOutlineActors.Add(OutlinerActor);
					}
					else
						OutlineActor->Destroy();
			}
	}
}