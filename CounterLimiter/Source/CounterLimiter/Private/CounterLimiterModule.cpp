#include "CounterLimiterModule.h"
#include "Patching/NativeHookManager.h"
#include "CL_CounterLimiter.h"
#include "Hologram/FGStandaloneSignHologram.h"
#include "FGIconLibrary.h"
#include "Buildables/FGBuildableWidgetSign.h"

DEFINE_LOG_CATEGORY(CounterLimiter_Log);

void FCounterLimiterModule::StartupModule() {
#if !WITH_EDITOR
	AFGStandaloneSignHologram* signHGCDO = GetMutableDefault<AFGStandaloneSignHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGStandaloneSignHologram::Construct, signHGCDO, [](AActor* constructedActor, AFGStandaloneSignHologram* self, TArray< AActor* >& out_children, FNetConstructionID netConstructionID)
		{
			AFGBuildable* snapped = self->GetSnappedBuilding();
			if (snapped)
			{
				auto counterBuildable = Cast<ACL_CounterLimiter>(snapped);
				if (counterBuildable)
				{
					AFGBuildableWidgetSign* widgetSign = Cast<AFGBuildableWidgetSign>(constructedActor);
					if (widgetSign)
					{
						counterBuildable->mAttachedSigns.AddUnique(widgetSign);
					}
				}
			}
		});
#endif
	//auto iconLib = UFGIconLibrary::Get();
	//auto icons = iconLib->GetIconData();
	//for (auto icon : iconLib->mIconData)
	//{
	//	icon.Hidden = true;
	//}
	//TSubclassOf< UFGIconLibrary > il = *Cast< TSubclassOf< UFGIconLibrary >>(iconLib);
	//UFGIconLibrary::CompileIconDatabase(il);
}


IMPLEMENT_GAME_MODULE(FCounterLimiterModule, CounterLimiter);