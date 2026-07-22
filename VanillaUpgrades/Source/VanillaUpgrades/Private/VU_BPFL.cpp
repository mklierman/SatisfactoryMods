


#include "VU_BPFL.h"
#include "FGPowerInfoComponent.h"

void UVU_BPFL::PowerStorageBeginPlay(AFGBuildablePowerStorage* powerStorage)
{
	powerStorage->BeginPlay();
}

void UVU_BPFL::PowerStorageRestart(AFGBuildablePowerStorage* powerStorage)
{
	if (powerStorage)
	{
		auto powerInfo = powerStorage->mPowerInfo;
		if (powerInfo)
		{
			powerInfo->InitializeBatteryInfo(powerStorage->mPowerStoreCapacity, powerStorage->mPowerInputCapacity);
			auto batteryInfo = powerStorage->mBatteryInfo;
			if (batteryInfo)
			{
				batteryInfo = powerInfo->GetBatteryInfo();
				float clampedPowerStore = FMath::Min(powerStorage->mPowerStoreCapacity, powerStorage->mPowerStore);
				powerStorage->mPowerStore = clampedPowerStore;
				batteryInfo->SetPowerStore(clampedPowerStore);
				batteryInfo->SetActive(true);
				powerStorage->UpdatePropertiesOnGameThread(EBatteryStatus::BS_Idle, powerStorage->CalculateIndicatorLevel());
			}
		}
	}
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
	if (auto fuelGen = Cast<AFGBuildableGeneratorFuel>(generator))
	{
		if (!fuelGen->HasFuel())
		{
			return;
		}
	}
	generator->Factory_StartPowerProduction_Implementation();
}

UObject* UVU_BPFL::GetCDOQuiet(UClass* Class)
{
	return Class->GetDefaultObject();
}
