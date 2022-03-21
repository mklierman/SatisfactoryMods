#include "FinerFoundationRotationModule.h"
#include "Patching/NativeHookManager.h"
#include "Hologram/FGFoundationHologram.h"

void FFinerFoundationRotationModule::StartupModule() {

#if !WITH_EDITOR
	AFGFoundationHologram* fdnCDO = GetMutableDefault<AFGFoundationHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGFoundationHologram::BeginPlay, fdnCDO, [](auto scope, AFGFoundationHologram* self)
		{
			self->mFineTuneRotationStep = 1;
		});
#endif
}


IMPLEMENT_GAME_MODULE(FFinerFoundationRotationModule, FinerFoundationRotation);