

#include "LBModularLoadBalancer_Hologram.h"
#include "LoadBalancersModule.h"
#include "FGColoredInstanceMeshProxy.h"
#include "FGProductionIndicatorInstanceComponent.h"
#include "LB_RCO.h"
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
	AActor* constructedActor = Super::Construct(out_children, netConstructionID);
	ALBBuild_ModularLoadBalancer* newBalancer = Cast<ALBBuild_ModularLoadBalancer>(constructedActor);
	if (newBalancer)
	{
		ALBBuild_ModularLoadBalancer* SnappedBalancer = Cast<ALBBuild_ModularLoadBalancer>(this->GetSnappedBuilding());

		if (SnappedBalancer)
		{
			newBalancer->SnappedBalancer = SnappedBalancer;
			newBalancer->GroupLeader = SnappedBalancer->GroupLeader;
			if (newBalancer->GroupLeader)
			{
				newBalancer->GroupLeader->GroupModules.AddUnique(newBalancer);
				for (auto conn : newBalancer->InputConnections)
				{
					newBalancer->GroupLeader->InputConnections.Add(conn);
				}
				for (auto conn : newBalancer->OutputMap)
				{
					newBalancer->GroupLeader->OutputMap.Add(conn);
				}
				for (auto conn : newBalancer->InputMap)
				{
					newBalancer->GroupLeader->InputMap.Add(conn);
				}
			}

			int32 currentInvSize = newBalancer->GroupLeader->Buffer->GetSizeLinear();
			newBalancer->GroupLeader->Buffer->Resize(currentInvSize + 2);
		}
		else
		{
			newBalancer->GroupLeader = newBalancer;
			newBalancer->GroupModules.AddUnique(newBalancer);
		}
	}

	for (auto Balancer : HologrammedBalancers)
	{
		UnHighlightBalancer(Balancer);
	}
	ForceNetUpdate();
	return constructedActor;
}

void ALBModularLoadBalancer_Hologram::Destroyed()
{
	UnHighlightAll();
	Super::Destroyed();
}

bool ALBModularLoadBalancer_Hologram::TrySnapToActor(const FHitResult& hitResult)
{
	ForceNetUpdate();
	bool SnapResult = Super::TrySnapToActor(hitResult);
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

			for (auto bal : SnappedBalancer->GroupLeader->GroupModules)
			{
				if (bal)
				{
					HighlightBalancer(bal);
					HologrammedBalancers.AddUnique(bal);
				}
			}
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
		//UE_LOG(LoadBalancers_Log, Display, TEXT("if (HasAuthority() && SnapResult)"));
		if (SnappedBalancer)
		{

			//UE_LOG(LoadBalancers_Log, Display, TEXT("if (SnappedBalancer)"));
			//TArray<ALBBuild_ModularLoadBalancer*> groupBalancers = SnappedBalancer->GroupLeader->GroupModules;
			//UE_LOG(LoadBalancers_Log, Display, TEXT("groupBalancers"));
			//SnappedBalancer->GroupLeader->OutputMap.GenerateKeyArray(groupBalancers);
			if (LastSnapped && SnappedBalancer != LastSnapped)
			{
				UnHighlightAll();
			}
			LastSnapped = SnappedBalancer;
			//UE_LOG(LoadBalancers_Log, Display, TEXT("LastSnapped = SnappedBalancer;"));
			for (auto bal : SnappedBalancer->GroupLeader->GroupModules)
			{
				if (bal)
				{
					//UE_LOG(LoadBalancers_Log, Display, TEXT("if (bal)"));
					HighlightBalancer(bal);
					HologrammedBalancers.AddUnique(bal);
				}
			}
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
	for (auto Balancer : HologrammedBalancers)
	{
		UnHighlightBalancer(Balancer);
	}
	//HologrammedBalancers.Empty();
}

void ALBModularLoadBalancer_Hologram::UnHighlightBalancer(ALBBuild_ModularLoadBalancer* Balancer)
{
		SetMeshInstanced(Balancer->Mesh, false);
		// replace materials for this mesh with hologram
		int Mats = Balancer->Mesh->GetNumMaterials();
		for (int i = 0; i < Mats; i++) {
			Balancer->Mesh->SetMaterial(i, Balancer->DefaultLoadBalancerMaterials[i]);
		}

		SetMeshInstanced(Balancer->Mesh, true);
}

void ALBModularLoadBalancer_Hologram::HighlightBalancer(ALBBuild_ModularLoadBalancer* Balancer)
{
		// code to call before swapping materials or after restoring materials

		SetMeshInstanced(Balancer->Mesh, false);
		int Mats = Balancer->Mesh->GetNumMaterials();
		for (int i = 0; i < Mats; i++)
		{
			Balancer->Mesh->SetMaterial(i, Balancer->TempLoadBalancerMaterial);
		}
		SetMeshInstanced(Balancer->Mesh, true);
}

void ALBModularLoadBalancer_Hologram::SetMeshInstanced(UMeshComponent* MeshComp, bool Instanced)
{
	auto StaticMeshProxy = Cast<UProxyInstancedStaticMeshComponent>(MeshComp);
	if (StaticMeshProxy) {
		StaticMeshProxy->SetInstanced(Instanced);
	}
	else {
		auto ColoredMeshProxy = Cast<UFGColoredInstanceMeshProxy>(MeshComp);
		if (ColoredMeshProxy) {
			ColoredMeshProxy->mBlockInstancing = !Instanced;
			ColoredMeshProxy->SetInstanced(Instanced);
		}
		else {
			auto ProdIndInst = Cast<UFGProductionIndicatorInstanceComponent>(MeshComp);
			if (ProdIndInst) {
				ProdIndInst->SetInstanced(Instanced);
			}
		}
	}
}
