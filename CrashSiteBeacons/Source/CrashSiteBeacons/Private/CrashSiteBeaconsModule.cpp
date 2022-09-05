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



void FCrashSiteBeaconsModule::StartupModule() {


#if !WITH_EDITOR
	UFGActorRepresentation* CDODropPod = GetMutableDefault<UFGActorRepresentation>();
	SUBSCRIBE_METHOD_VIRTUAL(UFGActorRepresentation::GetShouldShowInCompass, CDODropPod, [](auto& scope, const UFGActorRepresentation* self)
		{
			scope.Override(true);
		});


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