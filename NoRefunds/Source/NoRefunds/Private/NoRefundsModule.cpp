#include "NoRefundsModule.h"
#include "Equipment/FGBuildGunDismantle.h"
#include "Patching/NativeHookManager.h"
#include "FGInventoryComponent.h"

void FNoRefundsModule::StartupModule() {

#if !WITH_EDITOR
	TArray< FInventoryStack > refund = TArray< FInventoryStack >();
	SUBSCRIBE_METHOD(UFGBuildGunStateDismantle::GetDismantleRefund, [=](auto& scope, const UFGBuildGunStateDismantle* self)
		{
			TArray< FInventoryStack > refund = scope(self);
			refund.Empty();
			scope.Override(refund);
		});
#endif
}


IMPLEMENT_GAME_MODULE(FNoRefundsModule, NoRefunds);