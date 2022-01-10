// ILikeBanas

#include "CDO_BPLib.h"
#include "AI/FGAISystem.h"
#include "FGRailroadVehicleMovementComponent.h"
#include "Equipment/FGResourceScanner.h"
#include "Blueprint/UserWidget.h"
#include "Patching/BlueprintHookHelper.h"
#include "Patching/BlueprintHookManager.h"

void UCDO_BPLib::BindOnWidgetConstruct(const TSubclassOf<UUserWidget> WidgetClass, FOnWidgetCreated Binding) {
	if (!WidgetClass)
		return;
	UFunction* ConstructFunction = WidgetClass->FindFunctionByName(TEXT("Construct"));
	if (!ConstructFunction || ConstructFunction->IsNative())
	{
		return;
	}
	UBlueprintHookManager* HookManager = GEngine->GetEngineSubsystem<UBlueprintHookManager>();
	HookManager->HookBlueprintFunction(ConstructFunction, [Binding](FBlueprintHookHelper& HookHelper) {
		Binding.ExecuteIfBound(Cast<UUserWidget>(HookHelper.GetContext()));
		}, EPredefinedHookOffset::Return);
}
