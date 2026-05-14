

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Internationalization/Regex.h"
#include "FGBuildDescriptor.h"
#include "Buildables/FGBuildable.h"
#include "Uwu_BPFL.generated.h"

UCLASS()
class TOUWU_API UUwu_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "UWU")
	static void PatchDescriptors();

	UFUNCTION(BlueprintCallable, Category = "UWU")
	static void PatchBuildables();

	static FString RegexReplace(const FString& Input, const FString& Pattern, const FString& Replacement);
	static FString UwuifyString(const FString& Input);
};