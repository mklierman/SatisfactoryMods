#include "UniversalSoftClearanceModule.h"
#include "Hologram/FGFactoryBuildingHologram.h"
#include "FGClearanceComponent.h"
#include "Patching/NativeHookManager.h"

void FUniversalSoftClearanceModule::StartupModule() {
	AFGHologram* hCDO = GetMutableDefault<AFGHologram>();

#if !WITH_EDITOR
	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::SetupClearance, hCDO, [](auto scope, AFGHologram* self, class UFGClearanceComponent* clearanceComponent)
		{
			class UFGClearanceComponent* cc = clearanceComponent;
			cc->mIsSoftClearance = true;
			scope(self, cc);
		});
#endif
}


IMPLEMENT_GAME_MODULE(FUniversalSoftClearanceModule, UniversalSoftClearance);