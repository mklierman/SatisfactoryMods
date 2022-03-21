

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include <Runtime/UMG/Public/Components/Widget.h>
#include "CTB_BPFL.generated.h"

/**
 *
 */

UCLASS()
class CLOUDTOBUTT_API UCTB_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	//UFUNCTION(BlueprintCallable)
	//static void BindOnWidgetConstruct(const TSubclassOf<UUserWidget> WidgetClass, FOnWidgetConstruct Binding);


	UFUNCTION(BlueprintCallable)
		static TArray< UWidget* > GetAllWidgets(UUserWidget* Widget);

	UFUNCTION(BlueprintCallable)
		static void RegisterUserWidgetHook();
};
