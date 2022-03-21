


#include "CTB_BPFL.h"
#include "Patching/BlueprintHookHelper.h"
#include "Patching/BlueprintHookManager.h"
#include "Patching/NativeHookManager.h"
#include "Blueprint/WidgetTree.h"
#include "Subsystem/ModSubsystem.h"
#include "Subsystem/SubsystemActorManager.h"

//void UCTB_BPFL::BindOnWidgetConstruct(const TSubclassOf<UUserWidget> WidgetClass, FOnWidgetConstruct Binding)
//{
//	if (!WidgetClass)
//		return;
//	UFunction* ConstructFunction = WidgetClass->FindFunctionByName(TEXT("Construct"));
//	if (!ConstructFunction || ConstructFunction->IsNative())
//	{
//		return;
//	}
//	UBlueprintHookManager* HookManager = GEngine->GetEngineSubsystem<UBlueprintHookManager>();
//	HookManager->HookBlueprintFunction(ConstructFunction, [Binding](FBlueprintHookHelper& HookHelper) {
//		Binding.ExecuteIfBound(Cast<UUserWidget>(HookHelper.GetContext()));
//		}, EPredefinedHookOffset::Return);
//}

TArray<UWidget*> UCTB_BPFL::GetAllWidgets(UUserWidget* Widget)
{

	//TWeakObjectPtr MyObject = Widget;
	FText idk;
	TArray<UWidget*> Children;
	UWidgetTree* MyWidgetTree = Widget->WidgetTree;
	MyWidgetTree->GetAllWidgets(Children);
	return Children;
}

void UCTB_BPFL::RegisterUserWidgetHook()
{
#if !WITHEDITOR
	SUBSCRIBE_METHOD_AFTER(UUserWidget::Construct, [](UUserWidget* self)
		{
			//UWorld* WorldObject = self->GetWorld();
			//USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
			//ACTB_Subsystem* subsystem = SubsystemActorManager->GetSubsystemActor<ACTB_Subsystem>();

			//subsystem->OnWidgetConstruct.Broadcast(self);
		});
#endif
}

