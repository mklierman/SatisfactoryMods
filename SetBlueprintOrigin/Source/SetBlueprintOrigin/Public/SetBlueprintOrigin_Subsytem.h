

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "SetBlueprintOrigin_Subsytem.generated.h"

/**
 * 
 */
UCLASS()
class SETBLUEPRINTORIGIN_API ASetBlueprintOrigin_Subsytem : public AModSubsystem
{
	GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    bool ShouldUseHighest;

};
