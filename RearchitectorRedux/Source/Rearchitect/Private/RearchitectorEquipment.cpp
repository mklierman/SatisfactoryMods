
#include "RearchitectorEquipment.h"

#include "ArchitectorRCO.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "RearchitectorSubsystem.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include <Kismet/GameplayStatics.h>
#include "Buildables/FGBuildableConveyorBase.h"
#include "Buildables/FGBuildableConveyorAttachment.h"
#include "FGLightweightBuildableSubsystem.h"
#include "Buildables/FGBuildablePowerPole.h"
#include "Buildables/FGBuildablePipeBase.h"
#include "Buildables/FGBuildableRailroadTrack.h"


void ARearchitectorEquipment::PerformMove(const FVector& MoveAmount)
{
	if(!TargetManager.HasAnyTargets()) AddActor();
	
	TargetManager.DeltaMoveAllIndependent(MoveAmount);
}

void ARearchitectorEquipment::PerformRotate(const FVector& Rotation)
{
	if(!TargetManager.HasAnyTargets()) AddActor();
	
	TargetManager.DeltaRotate(Rotation);
}

void ARearchitectorEquipment::MoveToAimPosition(const FInputActionValue& ActionValue)
{
	if(ActionValue.Get<bool>())
	{
		if(!TargetManager.HasAnyTargets()) AddActor();

		bool Success;
		auto Data = GetTraceData(TraceDistance, TraceTypeQuery1, Success, true);
		TargetManager.MoveAllToPosition(Success ? Data.Location : Data.TraceEnd);	
	}
	else TargetManager.StopRecordingMoveAction();
}

void ARearchitectorEquipment::SetRotation(const FQuat& Rotation)
{
	if(!TargetManager.HasAnyTargets()) AddActor();

	TargetManager.SetRotationAllIndependent(Rotation);
}

void ARearchitectorEquipment::RandomRotation()
{
	if(!TargetManager.HasAnyTargets()) AddActor();

	TargetManager.SetRandomRotation();
}

void ARearchitectorEquipment::RotateToTarget(AActor* Target, EArchitectorAxis Axis)
{
	if(!TargetManager.HasAnyTargets()) AddActor();

	TargetManager.SetRotationToTarget(Target, Axis);
}

void ARearchitectorEquipment::RotateToPosition(const FVector& Position, EArchitectorAxis Axis)
{
	if(!TargetManager.HasAnyTargets()) AddActor();
	
	TargetManager.SetRotationToPosition(Position, Axis);
}

void ARearchitectorEquipment::PerformScale(const FVector& Scale)
{
	if(!TargetManager.HasAnyTargets()) AddActor();

	TargetManager.DeltaScale(Scale);
}

void ARearchitectorEquipment::RefreshOutline()
{
	ARearchitectorSubsystem::Self->RefreshOutline(TargetManager.GetTargetActors());
}

void ARearchitectorEquipment::HideOutlines()
{
	auto Outline = GetInstigatorCharacter()->GetOutline();
	auto OutlineActor = Outline->GetOwner();
	Outline->HideOutline(OutlineActor);
	auto OutlineState = Outline->GetOutlineStateForActor(OutlineActor);
	if (OutlineState)
	{
		auto OutlineProxies = OutlineState->OutlineProxies;
		for (auto Proxy : OutlineProxies)
		{
			Outline->RemoveOutlineProxy(OutlineActor, Proxy.Key);
		}
		auto InstancedOutlineProxies = OutlineState->InstancedOutlineProxies;
		for (auto InstProxy : InstancedOutlineProxies)
		{
			Outline->RemoveInstancedOutlineProxy(OutlineActor, InstProxy.Key);
		}
	}
	ARearchitectorSubsystem::Self->HideOutline(TargetManager.GetTargetActors());
}

void ARearchitectorEquipment::ShowOutlines()
{
	ARearchitectorSubsystem::Self->RefreshOutline(TargetManager.GetTargetActors());
}

void ARearchitectorEquipment::PreviewMassSelectActors()
{
	FVector Min, Max;
	TargetManager.GetTargetListBounds(Min, Max);
	ARearchitectorSubsystem::Self->UpdateMassSelectBounds(Min, Max, EOutlineColor::OC_NONE);

	TMap<AActor*, EOutlineColor> OutlineActors;
	for (const FOverlapInfo& Overlap : ARearchitectorSubsystem::Self->GetMassSelectOverlaps())
	{
		if(!FArchitectorToolTarget::IsValidTarget(Overlap.OverlapInfo)) continue;

		auto Target = FArchitectorToolTarget(Overlap.OverlapInfo);
		OutlineActors.Add(Target.Target, TargetManager.HasTarget(Target) ? EOutlineColor::OC_HOLOGRAM : EOutlineColor::OC_DISMANTLE);
	}

	ARearchitectorSubsystem::Self->RefreshOutline(OutlineActors);
}

void ARearchitectorEquipment::HideMassSelectActors()
{
	
}

void ARearchitectorEquipment::DismantleSelected()
{
	TargetManager.DismantleAndRefund();
}

FHitResult ARearchitectorEquipment::GetTraceData(double TraceDist, TEnumAsByte<ETraceTypeQuery> Channel, bool& Success, bool IgnoreTargetedActors)
{
	auto Camera = UGameplayStatics::GetPlayerCameraManager(this, 0);

	FVector Start = Camera->GetCameraLocation();
	FVector End = Camera->GetCameraLocation() + Camera->GetActorForwardVector() * TraceDist;
	FHitResult Result;

	FCollisionQueryParams TraceParams(TEXT("LineTraceSingle"), SCENE_QUERY_STAT_ONLY(KismetTraceUtils), false);
	TraceParams.bReturnPhysicalMaterial = true;
	TraceParams.bReturnFaceIndex = !UPhysicsSettings::Get()->bSuppressFaceRemapTable; // Ask for face index, as long as we didn't disable globally
	TraceParams.AddIgnoredActor(this);
	TraceParams.AddIgnoredActor(GetInstigatorCharacter());
	
	if(IgnoreTargetedActors) TargetManager.AddIgnoredActorsToTrace(TraceParams);

	Success = GetWorld()->LineTraceSingleByChannel(Result, Start, End, UEngineTypes::ConvertToCollisionChannel(Channel), TraceParams);
	return Result;
}

void ARearchitectorEquipment::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	bool Success;
	auto Data = GetTraceData(TraceDistance, TraceTypeQuery1, Success);
	auto Outline = GetInstigatorCharacter()->GetOutline();
	if (Outline)
	{
		auto OutlineActor = Outline->GetOwner();
		if (OutlineActor)
		{
			Outline->HideOutline(OutlineActor);

			if (!Success || !FArchitectorToolTarget::IsValidTarget(Data))
			{
				Outline->HideOutline(OutlineActor);
			}
			else
			{
				auto Target = FArchitectorToolTarget(Data);
				bool IsTargeted = !TargetManager.HasAnyTargets() || TargetManager.HasTarget(Target);
				Outline->ShowOutline(Target.Target, IsTargeted ? EOutlineColor::OC_HOLOGRAM : EOutlineColor::OC_SOFTCLEARANCE);
			}
		}
	}


	if(IsMassSelectActive)
	{
		Data = GetTraceData(TraceDistance, TraceTypeQuery1, Success);
		auto CurrentMassSelectEndPoint = Success ? Data.Location : Data.TraceEnd;
		
		ARearchitectorSubsystem::Self->UpdateMassSelectBounds(StartingMassSelectPoint, CurrentMassSelectEndPoint, IsMassSelectInDeselectMode ? EOutlineColor::OC_DISMANTLE : EOutlineColor::OC_HOLOGRAM);

		auto MassSelectOverlaps = ARearchitectorSubsystem::Self->GetMassSelectOverlaps();
		for (const FOverlapInfo& Overlap : MassSelectOverlaps)
		{
			if(!FArchitectorToolTarget::IsValidTarget(Overlap.OverlapInfo)) continue;
			
			auto Target = FArchitectorToolTarget(Overlap.OverlapInfo);
			if(IsMassSelectInDeselectMode) TargetManager.RemoveTarget(Target);
			else TargetManager.AddTarget(Target);
		}
	}

	RefreshOutline();
}

void ARearchitectorEquipment::InjectMappings()
{
	if(!IsLocallyControlled()) return;
		
	auto Player = Cast<AFGPlayerController>(GetInstigatorController());
	if(!Player) return;
		
		
	auto LocalPlayer = Player->GetLocalPlayer();
	if(!LocalPlayer) return;
		
		
	auto EnhancedInputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if(!EnhancedInputSystem) return;
		
		
	EnhancedInputSystem->AddMappingContext(ToolKeybinds, MappingContextPriority);
	EnhancedInputSystem->AddMappingContext(UIKeybinds, MappingContextPriority);
}

void ARearchitectorEquipment::EjectMappings(bool KeepInterfaceKeybinds)
{
	auto Player = Cast<AFGPlayerController>(GetInstigatorController());
	if(!Player) return;
		
	auto LocalPlayer = Player->GetLocalPlayer();
	if(!LocalPlayer) return;
		
	auto EnhancedInputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if(!EnhancedInputSystem) return;
		
	EnhancedInputSystem->RemoveMappingContext(ToolKeybinds);
	if(!KeepInterfaceKeybinds) EnhancedInputSystem->RemoveMappingContext(UIKeybinds);
}

void ARearchitectorEquipment::AddActor()
{
	bool Success;
	auto Target = GetTraceData(TraceDistance, ETraceTypeQuery::TraceTypeQuery1, Success);

	if(Success)
	{
		auto TargetData = FArchitectorToolTarget(Target);
		if(!TargetData.Target) return;

		if (TargetData.Target->IsA<AFGBuildableConveyorBase>())
		{
			return;
		}
		else if (TargetData.Target->IsA<AFGBuildableConveyorAttachment>())
		{
			return;
		}
		else if (TargetData.Target->IsA<AAbstractInstanceManager>())
		{
			return;
		}
		else if (TargetData.Target->IsA<AFGLightweightBuildableSubsystem>())
		{
			return;
		}
		else if (TargetData.Target->IsA<AFGBuildablePowerPole>())
		{
			return;
		}
		else if (TargetData.Target->IsA<AFGBuildablePipeBase>())
		{
			return;
		}
		else if (TargetData.Target->IsA<AFGBuildableRailroadTrack>())
		{
			return;
		}

		if(TargetManager.HasTarget(TargetData)) TargetManager.RemoveTarget(TargetData);
		else TargetManager.AddTarget(TargetData);
				
		RefreshOutline();
	}
	else ShowNotification(FText::FromString("No target in sight (too far?)"));
}
