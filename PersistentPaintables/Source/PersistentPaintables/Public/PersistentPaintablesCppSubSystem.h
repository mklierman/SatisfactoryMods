#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "Buildables/FGBuildable.h"
#include "PersistentPaintablesCppSubSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBuildEffectStarted, AFGBuildable*, Buildable, AActor*, Actor);

UCLASS()
class APersistentPaintablesCppSubSystem : public AModSubsystem
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable, Category = "Build Effect")
		FOnBuildEffectStarted OnBuildEffectStarted;
};

