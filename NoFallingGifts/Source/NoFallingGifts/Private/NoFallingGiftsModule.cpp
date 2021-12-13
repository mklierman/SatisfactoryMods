#include "NoFallingGiftsModule.h"
#include "Patching/NativeHookManager.h"
#include "FGItemPickup.h"

void FNoFallingGiftsModule::StartupModule() {

#if !WITH_EDITOR
	AFGItemPickup* ItemPickupCDO = GetMutableDefault<AFGItemPickup>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGItemPickup::BeginPlay, ItemPickupCDO, [](AFGItemPickup* self)
		{
			if (self->GetName().Contains("GiftBundle"))
			{
				self->Destroy();
			}
		});
#endif
}


IMPLEMENT_GAME_MODULE(FNoFallingGiftsModule, NoFallingGifts);