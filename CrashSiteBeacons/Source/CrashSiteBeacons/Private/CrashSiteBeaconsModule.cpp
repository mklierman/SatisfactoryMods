#include "CrashSiteBeaconsModule.h"
#include "CrashSiteBeaconsSubSystem.h"
#include "Subsystem/ModSubsystem.h"
#include "Subsystem/SubsystemActorManager.h"
#include "FGDropPod.h"
#include "Patching/NativeHookManager.h"
#include "Patching/BlueprintHookManager.h"
#include "Patching/BlueprintHookHelper.h"
#include "FGIconLibrary.h"
#include "FGActorRepresentation.h"
#include "FGMapManager.h"
#include "FGMapMarker.h"



void FCrashSiteBeaconsModule::StartupModule() {

#if !WITH_EDITOR
	SUBSCRIBE_METHOD(AFGDropPod::OpenDropPod, [](auto& scope, AFGDropPod* DropPod, AFGCharacterPlayer* player)
		{
			bool result = scope(DropPod, player);
			if (result)
			{
				auto mapManager = AFGMapManager::Get(DropPod->GetWorld());
				TArray<FMapMarker> markers;
				mapManager->GetMapMarkers(markers);
				if (markers.Num() > 0)
				{
					FVector podLoc = DropPod->GetActorLocation();
					for (auto marker : markers)
					{
						if (FVector::PointsAreNear(marker.Location, podLoc, 1.0))
						{
							mapManager->RemoveMapMarker(marker);
						}
					}
				}
			}
		});

	//UFGActorRepresentation* CDODropPod = GetMutableDefault<UFGActorRepresentation>();
	//SUBSCRIBE_METHOD_VIRTUAL(UFGActorRepresentation::GetShouldShowInCompass, CDODropPod, [](auto& scope, const UFGActorRepresentation* self)
	//	{
	//		scope.Override(true);
	//	});


	//UClass* SomeClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Interface/UI/Minimap/MapFilters/BPW_MapFilterCategories.BPW_MapFilterCategories_C"));
	//UFunction* SomeFunc = SomeClass->FindFunctionByName(TEXT("CanBeSeenOnCompass"));
	//UBlueprintHookManager* hooker = GetMutableDefault<UBlueprintHookManager>();

	//hooker->HookBlueprintFunction(SomeFunc, [=](FBlueprintHookHelper& helper) {
	//	//auto localStr = helper.GetLocalVarPtr<FUInt16Property>("StrVariable"); // getting the pointer to a local variable
	//	//auto output = GetOutVariablePtr<FBoolProperty>(TEXT("ReturnValue"));
	//	SetOutBool(helper.FramePointer, true, TEXT("ReturnValue"));
	//	}, EPredefinedHookOffset::Return);

	//

#endif
}


IMPLEMENT_GAME_MODULE(FCrashSiteBeaconsModule, CrashSiteBeacons);