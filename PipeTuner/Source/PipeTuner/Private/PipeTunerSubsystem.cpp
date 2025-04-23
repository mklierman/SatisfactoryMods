#include "PipeTunerSubsystem.h"
#include "Buildables/FGBuildablePipeline.h"
#include "Hologram/FGPipelinePumpHologram.h"
#include "Hologram/FGPipelineAttachmentHologram.h"
#include "FGFluidIntegrantInterface.h"
#include "Patching/BlueprintHookManager.h"
#include "FGGameMode.h"
#include "Patching/NativeHookManager.h"
#include "Modules/ModuleManager.h"

APipeTunerSubsystem::APipeTunerSubsystem()
{
#if !WITH_EDITOR
	//AFGPipelineAttachmentHologram* pah = GetMutableDefault<AFGPipelineAttachmentHologram>();
	//SUBSCRIBE_METHOD_VIRTUAL(AFGPipelineAttachmentHologram::BeginPlay, pah, [this](auto scope, AFGPipelineAttachmentHologram* self)
	//	{
	//		auto pumphg = Cast< AFGPipelinePumpHologram>(self);
	//		if (pumphg)
	//		{
	//			//pumphg->mMaxTraversalDistance = 50000;
	//			//pumphg->mMaxJunctionRecursions = 20;
	//			//pumphg->mOffsetEstimationBinaryDivisionCount = 16;
	//				SetTimer(pumphg);
	//		}
	//	});
#endif

	//UClass* SomeClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/-Shared/Holo_PipelinePump.Holo_PipelinePump_C"));
	//UFunction* SomeFunc = SomeClass->FindFunctionByName(TEXT("UpdateOrAddInstances"));
	//UBlueprintHookManager* HookManager = GEngine->GetEngineSubsystem<UBlueprintHookManager>();
	//HookManager->HookBlueprintFunction(SomeFunc, [](FBlueprintHookHelper& helper) {
	//	UObject* ctx = helper.GetContext(); // the object this function got called onto
	//	// do some nice stuff there
	//	helper.JumpToFunctionReturn();
	//	}, EPredefinedHookOffset::Start);
}


void APipeTunerSubsystem::SpawnPumpHG(AFGPipelinePumpHologram* PumpHolo, FTimerHandle handle)
{
	//if (PumpHolo)
	//{
	//	//FPumpHeadLiftLocationPath newPath;
	//	FPumpHeadLiftLocationPath currentPath = PumpHolo->mCurrentHeadLiftPath;
	//	if (currentPath.Spline)
	//	{
	//		//newPath.SetData(currentPath.Spline, currentPath.OffsetStart, currentPath.OffsetEnd, currentPath.ReverseSplineDirection);
	//		//const class USplineComponent* Spline = currentPath.Spline;
	//		 class USplineComponent* Spline;
	//		UObject* outer = currentPath.Spline->GetOuter();
	//		auto name = MakeUniqueObjectName(outer, USplineComponent::StaticClass());
	//		Spline = DuplicateObject(currentPath.Spline, outer, name);
	//		float OffsetStart = currentPath.OffsetStart;
	//		float OffsetEnd = currentPath.OffsetEnd;
	//		bool ReverseSplineDirection = currentPath.ReverseSplineDirection;
	//		FPumpHeadLiftLocationPath newPath;
	//		newPath.SetData(Spline, OffsetStart, OffsetEnd, ReverseSplineDirection);
	//		currentPath.AddNextPath(newPath);
	//	}
	//}
	//else
	//{
	//	PumpHolo->GetWorld()->GetTimerManager().ClearTimer(handle);
	//}
}

void APipeTunerSubsystem::SetTimer(AFGPipelinePumpHologram* PumpHolo)
{
		FTimerHandle PumpTimer;
		FTimerDelegate Delegate;
		Delegate.BindUObject(this, &APipeTunerSubsystem::SpawnPumpHG, PumpHolo, PumpTimer);
		PumpHolo->GetWorld()->GetTimerManager().SetTimer(PumpTimer, Delegate, 5.f, true, 5.f);
}

bool APipeTunerSubsystem::ShouldSave_Implementation() const
{
    return true;
}