#include "LBModularLoadBalancer_Hologram.h"
#include "FGColoredInstanceMeshProxy.h"
#include "FGProductionIndicatorInstanceComponent.h"
#include "Hologram/FGBuildableHologram.h"
#include "Kismet/KismetMathLibrary.h"
#include "LoadBalancersModule.h"
#include <Net/UnrealNetwork.h>
#include "Buildables/FGBuildableConveyorBelt.h"

//DEFINE_LOG_CATEGORY(LoadBalancers_Log);
#pragma optimize("", off)

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

void ALBModularLoadBalancer_Hologram::SetHologramLocationAndRotation(const FHitResult& hitResult)
{
	AActor* hitActor = hitResult.GetActor();
	ALBBuild_ModularLoadBalancer* cl = Cast <ALBBuild_ModularLoadBalancer>(hitActor);
	if (hitActor && cl)
	{
		FRotator addedRotation = FRotator(0, 0, 0);
		auto yVector = cl->GetActorRightVector();
		auto zVector = cl->GetActorUpVector();
		auto xVector = cl->GetActorForwardVector();
		if (this->GetRotationStep() != 0)
		{
			addedRotation.Yaw = mRotationAmount;
		}
		auto hitSide = GetHitSide(cl->GetActorTransform(), hitResult.ImpactNormal);
		switch (hitSide)
		{
		case EFoundationSide::FoundationRight:
			SetActorLocationAndRotation(cl->GetActorLocation() + (yVector * 200), cl->GetActorRotation() + addedRotation);
			this->mSnappedBuilding = cl;
			LastSnapped = cl;
			this->CheckValidPlacement();
			break;
		case EFoundationSide::FoundationLeft:
			SetActorLocationAndRotation(cl->GetActorLocation() + (yVector * -200), cl->GetActorRotation() + addedRotation);
			this->mSnappedBuilding = cl;
			LastSnapped = cl;
			this->CheckValidPlacement();
			break;
		case EFoundationSide::FoundationTop:
			SetActorLocationAndRotation(cl->GetActorLocation() + FVector(0, 0, 200), cl->GetActorRotation() + addedRotation);
			this->mSnappedBuilding = cl;
			LastSnapped = cl;
			this->CheckValidPlacement();
			break;
		case EFoundationSide::FoundationBottom:
			SetActorLocationAndRotation(cl->GetActorLocation() + FVector(0, 0, -200), cl->GetActorRotation() + addedRotation);
			this->mSnappedBuilding = cl;
			LastSnapped = cl;
			this->CheckValidPlacement();
			break;
		case EFoundationSide::FoundationFront:
			SetActorLocationAndRotation(cl->GetActorLocation() + (xVector * 200), cl->GetActorRotation() + addedRotation);
			this->mSnappedBuilding = cl;
			LastSnapped = cl;
			this->CheckValidPlacement();
			break;
		case EFoundationSide::FoundationBack:
			SetActorLocationAndRotation(cl->GetActorLocation() + (xVector * -200), cl->GetActorRotation() + addedRotation);
			this->mSnappedBuilding = cl;
			LastSnapped = cl;
			this->CheckValidPlacement();
			break;
		default:
			Super::SetHologramLocationAndRotation(hitResult);
			SetActorLocation(GetActorLocation() + FVector(0, 0, -100));
			LastSnapped = cl;
			break;
		}


		//if (hitResult.ImpactNormal.Z <= -0.75)
		//{
		//	SetActorLocationAndRotation(cl->GetActorLocation() + FVector(0, 0, -200), cl->GetActorRotation() + addedRotation);
		//}
		//else if (hitResult.ImpactNormal.Z >= 0.75)
		//{
		//	SetActorLocationAndRotation(cl->GetActorLocation() + FVector(0, 0, 200), cl->GetActorRotation() + addedRotation);
		//	this->mSnappedBuilding = cl;
		//	LastSnapped = cl;
		//	this->CheckValidPlacement();
		//}
		//else if ((hitResult.ImpactNormal * yVector).Y <= -0.05)
		//{
		//	// Snapping to left
		//	SetActorLocationAndRotation(cl->GetActorLocation() + (yVector * -200), cl->GetActorRotation() + addedRotation);
		//	this->mSnappedBuilding = cl;
		//	LastSnapped = cl;
		//	this->CheckValidPlacement();
		//}
		//else if ((hitResult.ImpactNormal * yVector).Y >= 0.05)
		//{
		//	// Snapping to right
		//	SetActorLocationAndRotation(cl->GetActorLocation() + (yVector * 200), cl->GetActorRotation() + addedRotation);
		//	this->mSnappedBuilding = cl;
		//	LastSnapped = cl;
		//	this->CheckValidPlacement();
		//}
		//else
		//{
		//	Super::SetHologramLocationAndRotation(hitResult);
		//	SetActorLocation(GetActorLocation() + FVector(0, 0, -100));
		//	LastSnapped = cl;
		//}
		//this->TrySnapToActor(hitResult);
	}
	else
	{
		Super::SetHologramLocationAndRotation(hitResult);
		SetActorLocation(GetActorLocation() + FVector(0, 0, -100));
		UnHighlightAll();
		LastSnapped = nullptr;
		ActiveGroupLeader = nullptr;
	}
}

void ALBModularLoadBalancer_Hologram::Destroyed()
{
	UnHighlightAll();
	Super::Destroyed();
}

bool ALBModularLoadBalancer_Hologram::TrySnapToActor(const FHitResult& hitResult)
{
	const bool SnapResult = Super::TrySnapToActor(hitResult);
	//SnapResult = false;
	if (!SnapResult)
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

			//SetActorRotation(Rot);
		}
	}
	else
	{
		SetActorLocation(GetActorLocation() + FVector(0, 0, -100));
		if (ActiveGroupLeader)
		{
			UnHighlightAll();
			LastSnapped = nullptr;
			ActiveGroupLeader = nullptr;
		}
	}

	return SnapResult;
}

bool ALBModularLoadBalancer_Hologram::CanNudgeHologram() const
{
	if (this->IsHologramLocked() && this->IsDisabled() == false)
	{
		return true;
	}
	return Super::CanNudgeHologram();
}

void ALBModularLoadBalancer_Hologram::ConfigureComponents(AFGBuildable* inBuildable) const
{
	Super::ConfigureComponents(inBuildable);

	if (mSnappedConveyor && inBuildable)
	{
		auto cl = Cast< ALBBuild_ModularLoadBalancer>(inBuildable);
		if (cl)
		{
			auto beltCustomizationData = mSnappedConveyor->Execute_GetCustomizationData(mSnappedConveyor);
			TArray< AFGBuildableConveyorBelt* > Belts = AFGBuildableConveyorBelt::Split(mSnappedConveyor, mSnappedConveyorOffset, false);
			if (Belts.Num() > 0)
			{
				for (auto Belt : Belts)
				{
					if (Belt->GetConnection0() && !Belt->GetConnection0()->IsConnected())
					{
						if (Belt->GetConnection0()->GetDirection() == EFactoryConnectionDirection::FCD_INPUT && !cl->MyOutputConnection->GetConnection())
						{
							Belt->GetConnection0()->SetConnection(cl->MyOutputConnection);
						}
						else if (Belt->GetConnection0()->GetDirection() == EFactoryConnectionDirection::FCD_OUTPUT && !cl->MyInputConnection->GetConnection())
						{
							Belt->GetConnection0()->SetConnection(cl->MyInputConnection);
						}
					}
					else if (Belt->GetConnection1() && !Belt->GetConnection1()->IsConnected() && !cl->MyOutputConnection->GetConnection())
					{
						if (Belt->GetConnection1()->GetDirection() == EFactoryConnectionDirection::FCD_INPUT)
						{
							Belt->GetConnection1()->SetConnection(cl->MyOutputConnection);
						}
						else if (Belt->GetConnection1()->GetDirection() == EFactoryConnectionDirection::FCD_OUTPUT && !cl->MyInputConnection->GetConnection())
						{
							Belt->GetConnection1()->SetConnection(cl->MyInputConnection);
						}
					}
					Belt->Execute_SetCustomizationData(Belt, beltCustomizationData);
				}
			}
		}
	}
}

void ALBModularLoadBalancer_Hologram::Scroll(int32 delta)
{
	//Super::SetScrollMode(EHologramScrollMode::HSM_ROTATE);
	if(GetSnappedBuilding() && Cast<ALBBuild_ModularLoadBalancer>(GetSnappedBuilding()))
	{
		//auto contr = Cast<APlayerController>(GetConstructionInstigator()->GetController());
		auto pContr = this->GetNetOwningPlayer()->GetPlayerController(GetWorld());
		if (pContr && pContr->IsInputKeyDown(EKeys::LeftControl))
		{
			//UE_LOG(LoadBalancers_Log, Display, TEXT("LeftControl"));
			mRotationAmount += delta * 5;
			SetScrollRotateValue(GetScrollRotateValue() + delta * 5);
			//Super::Scroll(delta);
		}
		else if (pContr && pContr->IsInputKeyDown(EKeys::LeftShift))
		{
			//UE_LOG(LoadBalancers_Log, Display, TEXT("LeftShift"));
			//Super::SetScrollMode(EHologramScrollMode::HSM_RAISE_LOWER);
			mRotationAmount += delta * 90;
			Super::Scroll(delta);
		}
		else
		{
			//SetScrollRotateValue(GetScrollRotateValue() + delta);
			mRotationAmount += delta * 45;
			Super::Scroll(delta);
		}
	}
	else
	{
		Super::Scroll(delta);
	}
}

EFoundationSide ALBModularLoadBalancer_Hologram::GetHitSide(FTransform hitTransform, FVector_NetQuantizeNormal hitNormal)
{
	auto xAxis = hitTransform.GetRotation().GetAxisX().Dot(hitNormal);
	auto yAxis = hitTransform.GetRotation().GetAxisY().Dot(hitNormal);
	auto zAxis = hitTransform.GetRotation().GetAxisZ().Dot(hitNormal);

	if (xAxis >= 0.7071)
		return EFoundationSide::FoundationFront;
	else if (xAxis <= -0.7071)
		return EFoundationSide::FoundationBack;
	else if (yAxis >= 0.7071)
		return EFoundationSide::FoundationRight;
	else if (yAxis <= -0.7071)
		return EFoundationSide::FoundationLeft;
	else if (zAxis >= 0.7071)
		return EFoundationSide::FoundationTop;
	else if (zAxis <= -0.7071)
		return EFoundationSide::FoundationBottom;

	return EFoundationSide::Invalid;
}


bool ALBModularLoadBalancer_Hologram::IsValidHitResult(const FHitResult& hitResult) const
{
	bool SuperBool = Super::IsValidHitResult(hitResult);
	SuperBool = true;
	// We clear the outline here if it invalid hit (in some cases it still hold the old outline because he switch instandly to Invalid)
	if (SuperBool && mOutlineSubsystem)
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
			auto components = actor->GetComponents();
			if (components.Num() > 0)
			{
				for (auto component : components)
				{
					if (component)
					{
						if (auto mesh = Cast<UFGColoredInstanceMeshProxy>(component))
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
			//auto comp = actor->GetComponentByClass(UFGColoredInstanceMeshProxy::StaticClass());
			//if (comp)
			//{
			//	if (auto mesh = Cast<UFGColoredInstanceMeshProxy>(comp))
			//	{
			//		mesh->SetMaterial(0, mMainMaterial);
			//		mesh->SetMaterial(1, mSecondaryMaterial);
			//		mesh->SetMaterial(2, mSymbolMaterial);
			//		mesh->SetInstanced(false);
			//		mesh->SetInstanced(true);
			//	}
			//}
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

			//Color stuff
			auto colors = Balancer->GroupLeader->GetCustomizationData_Implementation();
			Balancer->SetCustomizationData_Implementation(colors);
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
			auto components = OutlineActor->GetComponents();
			if (components.Num() > 0)
			{
				for (auto component : components)
				{
					if (component)
					{
						if (auto mesh = Cast<UFGColoredInstanceMeshProxy>(component))
						{
							mesh->SetMaterial(0, mHoloMaterial);
							mesh->SetMaterial(1, mHoloMaterial);
							mesh->SetInstanced(false);
							mesh->SetInstanced(true);
							mHighlightedActors.Add(OutlineActor);
						}
					}
				}
			}

			//auto comp = OutlineActor->GetComponentByClass(UFGColoredInstanceMeshProxy::StaticClass());
			//if (auto mesh = Cast<UFGColoredInstanceMeshProxy>(comp))
			//{
			//	mesh->SetMaterial(0, mHoloMaterial);
			//	mesh->SetMaterial(1, mHoloMaterial);
			//	mesh->SetInstanced(false);
			//	mesh->SetInstanced(true);
			//	mHighlightedActors.Add(OutlineActor);
			//}
			/*if (mOutlineSubsystem)
			{
				mOutlineSubsystem->CreateOutline(OutlineActor, true);
			}*/
		}
	}
}
#pragma optimize("", on)