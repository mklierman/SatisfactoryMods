#include "CrashSiteBeaconsModule.h"
#include "CrashSiteBeaconsSubSystem.h"
#include "Subsystem/ModSubsystem.h"
#include "Subsystem/SubsystemActorManager.h"
#include "FGDropPod.h"
#include "Patching/NativeHookManager.h"
#include "Patching/BlueprintHookManager.h"
#include "FGIconLibrary.h"
#include "FGActorRepresentation.h"
#include "FGMapManager.h"
#include "FGMapMarker.h"
#include <FGScannableSubsystem.h>

#pragma optimize("", off)


void FCrashSiteBeaconsModule::StartupModule() {

#if !WITH_EDITOR
	SUBSCRIBE_METHOD_AFTER(AFGScannableSubsystem::OnDropPodLooted, [this](auto result, class AFGDropPod* DropPod)
		{
				OnDropPodOpened(DropPod);
		});

	//UFGActorRepresentation* CDODropPod = GetMutableDefault<UFGActorRepresentation>();
	//SUBSCRIBE_METHOD_VIRTUAL(UFGActorRepresentation::GetShouldShowInCompass, CDODropPod, [](auto& scope, const UFGActorRepresentation* self)
	//	{
	//		scope.Override(true);
	//	});


	//UClass* SomeClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Interface/UI/Minimap/MapFilters/BPW_MapFilterCategories.BPW_MapFilterCategories_C"));
	//UFunction* SomeFunc = SomeClass->FindFunctionByName(TEXT("CanBeSeenOnCompass"));
	//UBlueprintHookManager* hooker = GetMutableDefault<UBlueprintHookManager>();

	//hooker->HookBlueprintFunction(SomeFunc, [this](FBlueprintHookHelper& helper) {
	//	//auto localStr = helper.GetLocalVarPtr<FUInt16Property>("StrVariable"); // getting the pointer to a local variable
	//	//auto output = GetOutVariablePtr<FBoolProperty>(TEXT("ReturnValue"));
	//	SetOutBool(helper.FramePointer, true, TEXT("ReturnValue"));
	//	}, EPredefinedHookOffset::Return);

	//

#endif
}

void FCrashSiteBeaconsModule::OnDropPodOpened(class AFGDropPod* pod)
{
	auto scannableSubsystem = AFGScannableSubsystem::Get(pod->GetWorld());
	auto availablePods = scannableSubsystem->GetAvailableDropPods();
	auto lootedPods = scannableSubsystem->mLootedDropPods;
	if (availablePods.Num() > 0 && lootedPods.Num() > 0)
	{
		auto mapManager = AFGMapManager::Get(pod->GetWorld());
		TArray<FMapMarker> markers;
		mapManager->GetMapMarkers(markers);

		for (auto aPod : availablePods)
		{
			if (lootedPods.Contains(aPod.ActorGuid))
			{
				auto aPodLoc = aPod.ActorLocation;
				for (auto marker : markers)
				{
					if (FVector::PointsAreNear(marker.Location, aPodLoc, 10.0))
					{
						mapManager->SetMarkerHighlighted(marker, false);
						mapManager->RemoveMapMarker(marker);
					}
				}
			}
		}
	}

}
#pragma optimize("", on)


IMPLEMENT_GAME_MODULE(FCrashSiteBeaconsModule, CrashSiteBeacons);