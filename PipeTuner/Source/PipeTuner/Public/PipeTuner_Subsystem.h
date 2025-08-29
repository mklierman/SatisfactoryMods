

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "Buildables/FGBuildablePipeline.h"
#include "PipeTuner_Subsystem.generated.h"

/**
 * 
 */
UCLASS()
class PIPETUNER_API APipeTuner_Subsystem : public AModSubsystem
{
	GENERATED_BODY()
public:
	TMap< TWeakObjectPtr< AFGBuildablePipeline>, float> PipeVolumes;
};
