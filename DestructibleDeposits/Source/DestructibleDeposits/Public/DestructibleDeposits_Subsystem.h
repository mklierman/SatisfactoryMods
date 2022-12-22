#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"

#include "DestructibleDeposits_Subsystem.generated.h"

UCLASS()
class DESTRUCTIBLEDEPOSITS_API ADestructibleDeposits_Subsystem : public AModSubsystem
{
	GENERATED_BODY()
public:
	ADestructibleDeposits_Subsystem();
	TScriptDelegate <FWeakObjectPtr> RadialDamage;

	UFUNCTION()
	void OnTakeRadialDamage(AActor* actor);
};
