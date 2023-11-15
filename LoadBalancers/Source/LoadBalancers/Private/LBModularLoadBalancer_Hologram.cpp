#include "LBModularLoadBalancer_Hologram.h"
#include "FGColoredInstanceMeshProxy.h"
#include "FGProductionIndicatorInstanceComponent.h"
#include "Hologram/FGBuildableHologram.h"
#include "Kismet/KismetMathLibrary.h"
#include "LoadBalancersModule.h"
#include <Net/UnrealNetwork.h>

//DEFINE_LOG_CATEGORY(LoadBalancers_Log);
//#pragma optimize("", off)

// Build step

// Step 1 - placement
// Step 2 - Stretch along axis player is looking in 1m increments until collision with line trace
// Step 2.5 - If collided with snap point don't go past it

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
			FRotator Rot = GetActorRotation();
			Rot.Yaw += 180 * GetScrollRotateValue();

			SetActorRotation(Rot);
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

void ALBModularLoadBalancer_Hologram::Scroll(int32 delta)
{
	//Super::SetScrollMode(EHologramScrollMode::HSM_ROTATE);
	if(GetSnappedBuilding() && Cast<ALBBuild_ModularLoadBalancer>(GetSnappedBuilding()))
	{
		auto contr = Cast<APlayerController>(GetConstructionInstigator()->GetController());
		if (contr && contr->IsInputKeyDown(EKeys::LeftControl))
		{
			UE_LOG(LoadBalancers_Log, Display, TEXT("LeftControl"));
			Super::Scroll(delta);
		}
		else if (contr && contr->IsInputKeyDown(EKeys::LeftShift))
		{
			UE_LOG(LoadBalancers_Log, Display, TEXT("LeftShift"));
			//Super::SetScrollMode(EHologramScrollMode::HSM_RAISE_LOWER);
			Super::Scroll(delta);
		}
		else
		{
			SetScrollRotateValue(GetScrollRotateValue() + delta);
		}
	}
	else
	{
		Super::Scroll(delta);
	}
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
	AActor* hitActor = hitResult.GetActor();
	ALBBuild_ModularLoadBalancer* cl = Cast <ALBBuild_ModularLoadBalancer>(hitActor);
	if (hitActor && cl)
	{
		return true;
	}

	return SuperBool;
}

void ALBModularLoadBalancer_Hologram::UnHighlightAll()
{
	for (auto actor : mHighlightedActors)
	{
		if (actor)
		{
			auto comp = actor->GetComponentByClass(UFGColoredInstanceMeshProxy::StaticClass());
			if (comp)
			{
				if (auto mesh = Cast<UFGColoredInstanceMeshProxy>(comp))
				{
					mesh->SetMaterial(0, mMainMaterial);
					mesh->SetMaterial(1, mSecondaryMaterial);
					mesh->SetMaterial(2, mSymbolMaterial);
					mesh->SetInstanced(false);
					mesh->SetInstanced(true);
				}
			}
		}
	}
	mHighlightedActors.Empty();
	//if (mOutlineSubsystem)
	//{
	//	mOutlineSubsystem->ClearOutlines(true);
	//}
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
		//mOutlineSubsystem->SetOutlineColor(mHoloColor, true);
		for (ALBBuild_ModularLoadBalancer* OutlineActor : actorsToOutline)
		{
			auto comp = OutlineActor->GetComponentByClass(UFGColoredInstanceMeshProxy::StaticClass());
			if (auto mesh = Cast<UFGColoredInstanceMeshProxy>(comp))
			{
				mesh->SetMaterial(0, mHoloMaterial);
				mesh->SetMaterial(1, mHoloMaterial);
				mesh->SetInstanced(false);
				mesh->SetInstanced(true);
				mHighlightedActors.Add(OutlineActor);
			}
			/*if (mOutlineSubsystem)
			{
				mOutlineSubsystem->CreateOutline(OutlineActor, true);
			}*/
		}
	}
}
//#pragma optimize("", on)