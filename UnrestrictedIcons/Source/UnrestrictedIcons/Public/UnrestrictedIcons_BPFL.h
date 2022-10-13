#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"
#include "FGIconLibrary.h"
#include "UnrestrictedIcons_BPFL.generated.h"

UCLASS()
class UNRESTRICTEDICONS_API UUnrestrictedIcons_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	public:

	UFUNCTION(BlueprintCallable)
	static void AddEnums();

	UFUNCTION(BlueprintCallable)
	static int GetModsCategoryEnumValue();

	UFUNCTION(BlueprintCallable)
	static int GetImportedCategoryEnumValue();

	UFUNCTION(BlueprintCallable)
	static int GetExtraCategoryEnumValue();
};
