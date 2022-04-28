#include "LBModularLoadBalancer_Hologram.h"
#include "FGColoredInstanceMeshProxy.h"
#include "FGProductionIndicatorInstanceComponent.h"
#include "LoadBalancersModule.h"

//DEFINE_LOG_CATEGORY(LoadBalancers_Log);

void ALBModularLoadBalancer_Hologram::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALBModularLoadBalancer_Hologram, LastSnapped);
	DOREPLIFETIME(ALBModularLoadBalancer_Hologram, ActiveGroupLeader);
}

void ALBModularLoadBalancer_Hologram::BeginPlay()
{
	mOutlineSubsystem = ALBOutlineSubsystem::Get(GetWorld());
	Super::BeginPlay();
}

AActor* ALBModularLoadBalancer_Hologram::Construct(TArray<AActor*>& out_children, FNetConstructionID netConstructionID)
{
	UnHighlightAll();
	return Super::Construct(out_children, netConstructionID);
}

void ALBModularLoadBalancer_Hologram::Destroyed()
{
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
			if (ActiveGroupLeader != SnappedBalancer->GroupLeader)
			{
				UnHighlightAll();
			}

			LastSnapped = SnappedBalancer;
			ActiveGroupLeader = SnappedBalancer->GroupLeader;

			HighlightAll(SnappedBalancer->GetGroupModules());
		}
	}
	else
	{
		if (ActiveGroupLeader)
		{
			UnHighlightAll();
			LastSnapped = nullptr;
			ActiveGroupLeader = nullptr;
		}
	}

	return SnapResult;
}

bool ALBModularLoadBalancer_Hologram::IsValidHitResult(const FHitResult& hitResult) const
{
	bool SuperBool = Super::IsValidHitResult(hitResult);

	// We clear the outline here if it invalid hit (in some cases it still hold the old outline because he switch instandly to Invalid)
	if (!SuperBool && mOutlineSubsystem)
	{
		if (mOutlineSubsystem->HasAnyOutlines())
		{
			mOutlineSubsystem->ClearOutlines();
		}
	}

	return SuperBool;
}

void ALBModularLoadBalancer_Hologram::UnHighlightAll()
{
	if (mOutlineSubsystem)
	{
		mOutlineSubsystem->ClearOutlines(true);
	}
}

void ALBModularLoadBalancer_Hologram::ConfigureActor(AFGBuildable* inBuildable) const
{
	if (ALBBuild_ModularLoadBalancer* Balancer = Cast<ALBBuild_ModularLoadBalancer>(inBuildable))
	{
		if (LastSnapped)
		{
			Balancer->GroupLeader = LastSnapped->GroupLeader;
		}
		else
		{
			Balancer->GroupLeader = Balancer;
		}
	}

	Super::ConfigureActor(inBuildable);
}

void ALBModularLoadBalancer_Hologram::HighlightAll(TArray<ALBBuild_ModularLoadBalancer*> actorsToOutline)
{
	if (actorsToOutline.Num() <= 0)
	{
		return;
	}

	if (ActiveGroupLeader)
	{
		mOutlineSubsystem->SetOutlineColor(mHoloColor, true);
		for (ALBBuild_ModularLoadBalancer* OutlineActor : actorsToOutline)
		{
			if (mOutlineSubsystem)
			{
				mOutlineSubsystem->CreateOutline(OutlineActor, true);
			}
		}
	}
}