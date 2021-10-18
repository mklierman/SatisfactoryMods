// ILikeBanas

#pragma once
#include "FGCharacterPlayer.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PEDFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class PERFORMANCEENHANCINGDRUGS_API UPEDFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		static void SetRadiationImmunity(AFGCharacterPlayer* Player, float Amount);
};
