


#include "VU_BPFL.h"

void UVU_BPFL::PowerStorageBeginPlay(AFGBuildablePowerStorage* powerStorage)
{
	powerStorage->BeginPlay();
}

void UVU_BPFL::CalculateProductionCycleTime(AFGBuildableResourceExtractor* extractor)
{
	extractor->CalculateProductionCycleTime();
}

void UVU_BPFL::FuelGenBeginPlay(AFGBuildableGeneratorFuel* fuelGen)
{
	fuelGen->BeginPlay();
}

void UVU_BPFL::FactoryBeginPlay(AFGBuildableFactory* factory)
{
	factory->BeginPlay();
}

void UVU_BPFL::StartPowerProduction(AFGBuildableGenerator* generator)
{
	generator->Factory_StartPowerProduction_Implementation();
}
