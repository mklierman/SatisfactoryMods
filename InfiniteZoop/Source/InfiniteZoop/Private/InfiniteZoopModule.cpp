#include "InfiniteZoopModule.h"
#include "Hologram/FGFactoryBuildingHologram.h"
#include "Patching/NativeHookManager.h"

void FInfiniteZoopModule::StartupModule() {
#if !WITH_EDITOR
	AFGHologram* hCDO = GetMutableDefault<AFGHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::BeginPlay, hCDO, [](auto scope, AFGHologram* self)
		{
			AFGFactoryBuildingHologram* bhg = Cast<AFGFactoryBuildingHologram>(self);
			if (bhg)
			{
				bhg->mMaxZoopAmount = 9999;
			}
		});
#endif
}


IMPLEMENT_GAME_MODULE(FInfiniteZoopModule, InfiniteZoop);