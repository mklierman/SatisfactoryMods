

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "CP_Subsystem.generated.h"

/**
 *
 */
UCLASS()
class CONSTRUCTIONPREFERENCES_API ACP_Subsystem : public AModSubsystem
{
	GENERATED_BODY()
public:
		UPROPERTY(BlueprintReadWrite, Category = "Conveyor Belt")
		TSubclassOf< class UFGRecipe > mConveyorPoleRecipe;
};
