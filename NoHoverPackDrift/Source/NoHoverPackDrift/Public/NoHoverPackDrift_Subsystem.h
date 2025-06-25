

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "NoHoverPackDrift_Subsystem.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FHoverPackDefaultsStruct
{
	GENERATED_BODY()

public:
	UPROPERTY()
	float Speed;
	UPROPERTY()
	float Acceleration;
	UPROPERTY()
	float Friction;
	UPROPERTY()
	float Radius;
	UPROPERTY()
	float Sprint;
};

UCLASS()
class NOHOVERPACKDRIFT_API ANoHoverPackDrift_Subsystem : public AModSubsystem
{
	GENERATED_BODY()
public:

	ANoHoverPackDrift_Subsystem();
	
	UPROPERTY()
	TMap<UClass*, FHoverPackDefaultsStruct> HoverpackMap;
};
