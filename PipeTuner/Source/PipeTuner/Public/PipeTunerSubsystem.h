#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "Buildables/FGBuildablePipeline.h"
#include "Hologram/FGPipelinePumpHologram.h"
#include "Hologram/FGPipelineAttachmentHologram.h"

#include "PipeTunerSubsystem.generated.h"

UCLASS()
class PIPETUNER_API APipeTunerSubsystem : public AModSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()
public:
	APipeTunerSubsystem();

	virtual bool ShouldSave_Implementation() const override;
	
	FTimerManager MyTimerManager;

	UFUNCTION()
	void SpawnPumpHG(AFGPipelinePumpHologram* PumpHolo, FTimerHandle handle);

	UFUNCTION()
	void SetTimer(AFGPipelinePumpHologram* PumpHolo);

	TArray<AFGPipelinePumpHologram*> ActiveHolograms;

	TMap<AFGPipelinePumpHologram*, FTimerHandle> ActiveHologramTimers;
};
