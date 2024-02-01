#include "DaisyChainEverythingModule.h"
#include "Patching/NativeHookManager.h"
#include "Buildables/FGBuildable.h"
#include "Components/ActorComponent.h"
#include "FGPowerConnectionComponent.h"
#include "Buildables/FGBuildablePowerPole.h"
#include "SessionSettings/SessionSettingsManager.h"
#include "DCE_ConfigurationStruct.h"

DEFINE_LOG_CATEGORY(DaisyChainEverythingModule_Log);

void FDaisyChainEverythingModule::StartupModule() {
	AFGBuildable* bCDO = GetMutableDefault<AFGBuildable>();
#if !WITH_EDITOR
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildable::BeginPlay, bCDO, []( AFGBuildable* self)
		{
			USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			auto minConnections = (int)SessionSettings->GetFloatOptionValue("DaisyChainEverything.MinConnections");
			//auto config = FDCE_ConfigurationStruct::GetActiveConfig(self->GetWorld());
			auto IgnorePowerPoles = SessionSettings->GetBoolOptionValue("DaisyChainEverything.IgnorePowerPolesBool");
			if (IgnorePowerPoles)
			{
				auto PowerPole = Cast< AFGBuildablePowerPole>(self);
				if (PowerPole)
				{
					return;
				}
			}
			//auto MinNumConn = config.GlobalSetToInt;
			TInlineComponentArray<UFGPowerConnectionComponent*> powerConns(self);
			for (int i = 0; i < powerConns.Num(); i++)
			{
				if (powerConns[i]->mMaxNumConnectionLinks < minConnections)
				{
					powerConns[i]->mMaxNumConnectionLinks = minConnections;
				}
			}
		});
#endif
}


IMPLEMENT_GAME_MODULE(FDaisyChainEverythingModule, DaisyChainEverything);