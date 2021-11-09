#include "CrashSiteBeaconsModule.h"
#include "Patching/NativeHookManager.h"
#include "Patching/BlueprintHookManager.h"
#include "Patching/BlueprintHookHelper.h"
#include "CrashSiteBeaconsSubSystem.h"
#include "Subsystem/ModSubsystem.h"
#include "Subsystem/SubsystemActorManager.h"
#include "FGDropPod.h"

void FCrashSiteBeaconsModule::StartupModule() {
#if !WITH_EDITOR
	//AFGDropPod* CDODropPod = GetMutableDefault<AFGDropPod>();
	//SUBSCRIBE_METHOD_VIRTUAL(AFGDropPod::OnOpened, CDODropPod, [](auto& scope, AFGDropPod* self)
	//	{
	//		UWorld* WorldObject = self->GetWorld();
	//		USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
	//		ACrashSiteBeaconsSubSystem* subsystem = SubsystemActorManager->GetSubsystemActor<ACrashSiteBeaconsSubSystem>();
	//		subsystem->OnDropPodOpened.Broadcast(self);
	//	});

	//UClass* SomeClass = GetMutableDefault<AFGDropPod>()->GetClass();
	//UFunction* SomeFunc = SomeClass->FindFunctionByName(TEXT("OnOpened"));
	//UBlueprintHookManager* hooker = GetMutableDefault<UBlueprintHookManager>();
	//hooker->HookBlueprintFunction(SomeFunc, [](FBlueprintHookHelper& helper) {
	//	UObject* ctx = helper.GetContext(); // the object this function got called onto
	//	// do some nice stuff there
	//	AFGDropPod* self = Cast<AFGDropPod>(ctx);
	//	UWorld* WorldObject = self->GetWorld();
	//	USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
	//	ACrashSiteBeaconsSubSystem* subsystem = SubsystemActorManager->GetSubsystemActor<ACrashSiteBeaconsSubSystem>();
	//	subsystem->OnDropPodOpened.Broadcast(self);
	//	}, EPredefinedHookOffset::Return);
#endif
}


IMPLEMENT_GAME_MODULE(FCrashSiteBeaconsModule, CrashSiteBeacons);