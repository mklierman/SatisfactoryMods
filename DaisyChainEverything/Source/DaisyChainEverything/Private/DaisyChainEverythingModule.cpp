#include "DaisyChainEverythingModule.h"
#include "Patching/NativeHookManager.h"
#include "Buildables/FGBuildable.h"
#include "Components/ActorComponent.h"
#include "FGPowerConnectionComponent.h"
#include "Buildables/FGBuildablePowerPole.h"
#include "DCE_ConfigurationStruct.h"

DEFINE_LOG_CATEGORY(DaisyChainEverythingModule_Log);

void FDaisyChainEverythingModule::StartupModule() {
#if !WITH_EDITOR
	AFGBuildable* bCDO = GetMutableDefault<AFGBuildable>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildable::BeginPlay, bCDO, []( AFGBuildable* self)
		{
			auto config = FDCE_ConfigurationStruct::GetActiveConfig(self->GetWorld());
			auto IgnorePowerPoles = config.IgnorePowerPolesBool;
			if (IgnorePowerPoles)
			{
				auto PowerPole = Cast< AFGBuildablePowerPole>(self);
				if (PowerPole)
				{
					return;
				}
			}
			auto MinNumConn = config.GlobalSetToInt;
			TInlineComponentArray<UFGPowerConnectionComponent*> powerConns(self);
			for (int i = 0; i < powerConns.Num(); i++)
			{
				if (powerConns[i]->mMaxNumConnectionLinks < MinNumConn)
				{
					powerConns[i]->mMaxNumConnectionLinks = MinNumConn;
				}
			}
		});
#endif
}


IMPLEMENT_GAME_MODULE(FDaisyChainEverythingModule, DaisyChainEverything);