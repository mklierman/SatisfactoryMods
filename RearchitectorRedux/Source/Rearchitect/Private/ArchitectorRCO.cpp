#include "ArchitectorRCO.h"

#include "ActorUtilities.h"
#include "FGPlayerState.h"

void UArchitectorRCO::ApplyModifyDataToAll_Implementation(const TArray<FArchitectorToolTarget>& Targets, const FActorTransformModifyData& TransformData)
{
	Multicast_ApplyModifyDataToAll(Targets, TransformData);
}

void UArchitectorRCO::Multicast_ApplyModifyDataToAll_Implementation(const TArray<FArchitectorToolTarget>& Targets, const FActorTransformModifyData& TransformData)
{
	for (const FArchitectorToolTarget& Target : Targets) PerformActionOnTarget(Target, TransformData);
}

void UArchitectorRCO::ApplyModifyDataOnEach_Implementation(const TArray<FTargetModifyData>& Data)
{
	Multicast_ApplyModifyDataOnEach_Implementation(Data);
}

void UArchitectorRCO::Multicast_ApplyModifyDataOnEach_Implementation(const TArray<FTargetModifyData>& Datas)
{
	for (const FTargetModifyData& Data : Datas) PerformActionOnTarget(Data.Target, Data.TransformData);
}

void UArchitectorRCO::ApplyTransformOnEach_Implementation(const TArray<FTargetTransformData>& Data)
{
	Multicast_ApplyTransformOnEach_Implementation(Data);
}

void UArchitectorRCO::Multicast_ApplyTransformOnEach_Implementation(const TArray<FTargetTransformData>& Data)
{
	for (const FTargetTransformData& TargetTransformData : Data)
	{
		if(!TargetTransformData.Target.Target) continue;
		
		const auto& ActorT = TargetTransformData.ActorTransform;
		const auto& AbstT = TargetTransformData.AbstractTransform;
		const auto& Target = TargetTransformData.Target;

		Target.Target->SetActorTransform(ActorT);
		// Broke by 1.1
		if (Target.IsAbstract)
		{
			auto handle = Target.GenerateInstanceHandle();// .UpdateTransform(AbstT);
		}
	}
}

void UArchitectorRCO::DismantleAndRefund_Implementation(AFGPlayerState* Player, const TArray<FArchitectorToolTarget>& Targets)
{
	TArray<FInventoryStack> Refund;
	const bool NoBuildCost = Player->GetPlayerRules().NoBuildCost;

	for (const FArchitectorToolTarget& Target : Targets)
	{
		auto Buildable = Cast<AFGBuildable>(Target.Target);
		if(!Buildable) continue;

		IFGDismantleInterface::Execute_GetDismantleRefund(Buildable, Refund, NoBuildCost);
		IFGDismantleInterface::Execute_Dismantle(Buildable);
	}

	Cast<AFGCharacterPlayer>(Player->GetPawn())->GetInventory()->AddStacks(Refund);
}

void UArchitectorRCO::PerformActionOnTarget(const FArchitectorToolTarget& Target, const FActorTransformModifyData& TransformData)
{
	if(!Target.Target) return;
	
	FInstanceHandle InstanceHandle;
	//Make target actor movable
	if(auto Root = Target.Target->GetRootComponent()) Root->SetMobility(EComponentMobility::Movable);
	if(Target.IsAbstract)
	{
		InstanceHandle = Target.GenerateInstanceHandle();
		// Broke by 1.1
		//InstanceHandle.GetInstanceComponent()->SetMobility(EComponentMobility::Movable);
	}
	
	for(auto& Component : TInlineComponentArray<UFGColoredInstanceMeshProxy*>( Target.Target ))
	{
		UActorUtilities::RefreshInstanceHandle(Component);
	}

	//Move
	if(TransformData.Move.IsUsed)
	{
		//Transform actor
		auto Transform = Target.Target->GetTransform();
		auto& PositionValue = TransformData.Move.Value;
			
		if(TransformData.Move.UseAsSetAction) Transform.SetLocation(PositionValue);
		else Transform.AddToTranslation(PositionValue);
			
		Target.Target->SetActorTransform(Transform);
		// Broke by 1.1
		//Target.Target->CachedActorTransform = Transform;
		

		//Transform abstract instance
		if(Target.IsAbstract)
		{
			InstanceHandle.GetInstanceComponent()->GetInstanceTransform(InstanceHandle.GetHandleID(), Transform);

			if(TransformData.Move.UseAsSetAction) Transform.SetLocation(PositionValue);
			else Transform.AddToTranslation(PositionValue);
			// Broke by 1.1
			//InstanceHandle.UpdateTransform(Transform);
		}
	}

	//Rotate
	if(TransformData.Rotate.IsUsed)
	{
		//Transform actor
		auto NewActorRotation = TransformData.Rotate.UseAsSetAction
		? TransformData.Rotate.Value
		: TransformData.Rotate.Value * Target.Target->GetActorQuat();
			
		Target.Target->SetActorRotation(NewActorRotation);

		//Transform abstract instance
		if(Target.IsAbstract)
		{
			FTransform Transform;
			InstanceHandle.GetInstanceComponent()->GetInstanceTransform(InstanceHandle.GetHandleID(), Transform);

			auto NewInstanceRotation = TransformData.Rotate.UseAsSetAction
			? TransformData.Rotate.Value
			: TransformData.Rotate.Value * Transform.GetRotation();

			Transform.SetRotation(NewInstanceRotation);
			// Broke by 1.1
			//InstanceHandle.UpdateTransform(Transform);
		}
	}

	//Scale
	if(TransformData.Scale.IsUsed)
	{
		//Transform actor
		auto NewActorScale = TransformData.Scale.UseAsSetAction
		? TransformData.Scale.Value
		: Target.Target->GetActorScale3D() + TransformData.Scale.Value;

		Target.Target->SetActorScale3D(NewActorScale);

		//Transform abstract instance
		if(Target.IsAbstract)
		{
			FTransform Transform;
			InstanceHandle.GetInstanceComponent()->GetInstanceTransform(InstanceHandle.GetHandleID(), Transform);

			auto NewInstanceScale = TransformData.Scale.UseAsSetAction
			? TransformData.Scale.Value
			: Transform.GetScale3D() + TransformData.Scale.Value;

			Transform.SetScale3D(NewInstanceScale);
			// Broke by 1.1
			//InstanceHandle.UpdateTransform(Transform);
		}
	}
}
