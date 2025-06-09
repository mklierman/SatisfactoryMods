


#include "VU_BPFL.h"

void UVU_BPFL::PowerStorageBeginPlay(AFGBuildablePowerStorage* powerStorage)
{
	powerStorage->BeginPlay();
}

void UVU_BPFL::CalculateProductionCycleTime(AFGBuildableResourceExtractor* extractor)
{
	extractor->CalculateProductionCycleTime();
}
