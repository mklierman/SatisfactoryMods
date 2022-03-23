

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

void ALBModularLoadBalancer_Hologram::BeginPlay()
{
	Super::BeginPlay();
}

bool ALBModularLoadBalancer_Hologram::TrySnapToActor(const FHitResult& hitResult)
{
	ForceNetUpdate();
	const bool SnapResult = Super::TrySnapToActor(hitResult);
	if (SnapResult)
	{
		ALBBuild_ModularLoadBalancer* SnappedBalancer = Cast<ALBBuild_ModularLoadBalancer>(this->GetSnappedBuilding());
		if (SnappedBalancer)
		{
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
		}
	}
	ForceNetUpdate();
	return SnapResult;
}

void ALBModularLoadBalancer_Hologram::Server_SnapStuff(bool SnapResult)
{
	this->SetOwner(GetWorld()->GetFirstPlayerController());
	if (SnapResult)
	{
		ALBBuild_ModularLoadBalancer* SnappedBalancer = Cast<ALBBuild_ModularLoadBalancer>(this->GetSnappedBuilding());
		if (SnappedBalancer)
		{
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
		}
	}
}

void ALBModularLoadBalancer_Hologram::UnHighlightAll()
{
	if(UFGOutlineComponent* OL = UFGOutlineComponent::Get(GetWorld()))
		OL->HideOutline();
}

void ALBModularLoadBalancer_Hologram::ConfigureActor(AFGBuildable* inBuildable) const
{
	if(ALBBuild_ModularLoadBalancer* Balancer = Cast<ALBBuild_ModularLoadBalancer>(inBuildable))
		if(inBuildable && LastSnapped)
		{
			Balancer->GroupLeader = LastSnapped;
			Balancer->ApplyGroupModule(Balancer);
			UE_LOG(LoadBalancers_Log, Warning, TEXT("BUILD > Apply GroupLeader > LastSnapped"))
		}
		else
		{
			Balancer->GroupLeader = Balancer;
			Balancer->ApplyGroupModule(Balancer);
			UE_LOG(LoadBalancers_Log, Warning, TEXT("BUILD > Is Now Group Leader"))
		}
	
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
	if (actorsToOutline != HologrammedBalancers)
	{
		UFGOutlineComponent* OL = UFGOutlineComponent::Get(GetWorld());

		if(OL && GetSnappedBuilding())
		{
			OL->ShowDismantlePendingMaterial(TArray<AActor*>(actorsToOutline));

			UStaticMesh* Mesh = actorsToOutline[0]->FindComponentByClass<UFGColoredInstanceMeshProxy>()->GetStaticMesh();
			for (AActor* OutlineActor : OL->mActiveMultiOutlineActors)
			{
				UStaticMeshComponent* Component = OutlineActor->FindComponentByClass<UStaticMeshComponent>();
				if(Component)
				{
					Component->SetStaticMesh(Mesh);
					UE_LOG(LoadBalancers_Log, Display, TEXT("SetStaticMesh"));
				}
			}
		}
		
		HologrammedBalancers = actorsToOutline;
	}
}