#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"

#include "FGAtmosphereVolume.h"
#include "Atmosphere/UFGBiome.h"
#include "AE_Subsystem.generated.h"

UCLASS()
class ATMOSPHEREEDITOR_API AAE_Subsystem : public AModSubsystem
{
	GENERATED_BODY()
public:
	TArray<AFGAtmosphereVolume> AtmosphereActors;

};
