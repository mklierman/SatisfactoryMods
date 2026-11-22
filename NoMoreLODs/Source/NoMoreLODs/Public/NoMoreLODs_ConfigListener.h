#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NoMoreLODs_ConfigListener.generated.h"

class FNoMoreLODsModule;
class UConfigProperty;

UCLASS()
class UNoMoreLODsConfigListener final : public UObject {
	GENERATED_BODY()

public:
	void Bind(FNoMoreLODsModule* newModule, UConfigProperty* newProperty);
	void Unbind();

private:
	UFUNCTION()
	void HandlePropertyValueChanged();

	FNoMoreLODsModule* module = nullptr;

	UPROPERTY()
	TObjectPtr<UConfigProperty> property;
};
