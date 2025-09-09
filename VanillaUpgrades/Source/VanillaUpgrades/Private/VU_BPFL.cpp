


#include "VU_BPFL.h"
#include "FGPowerInfoComponent.h"

void UVU_BPFL::PowerStorageBeginPlay(AFGBuildablePowerStorage* powerStorage)
{
	powerStorage->BeginPlay();
}

void UVU_BPFL::PowerStorageRestart(AFGBuildablePowerStorage* powerStorage)
{
	powerStorage->mPowerInfo->InitializeBatteryInfo(powerStorage->mPowerStoreCapacity, powerStorage->mPowerInputCapacity);
	powerStorage->mBatteryInfo = powerStorage->mPowerInfo->GetBatteryInfo();
	float clampedPowerStore = FMath::Min(powerStorage->mPowerStoreCapacity, powerStorage->mPowerStore);
	powerStorage->mPowerStore = clampedPowerStore;
	powerStorage->mBatteryInfo->SetPowerStore(clampedPowerStore);
	powerStorage->mBatteryInfo->SetActive(true);
	powerStorage->UpdatePropertiesOnGameThread(EBatteryStatus::BS_Idle, powerStorage->CalculateIndicatorLevel());
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
