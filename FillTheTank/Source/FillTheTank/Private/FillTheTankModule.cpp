#include "FillTheTankModule.h"
#include "Patching/NativeHookManager.h"
#include "WheeledVehicles/FGWheeledVehicle.h"

DEFINE_LOG_CATEGORY(FillTheTank_Log);
void FFillTheTankModule::StartupModule() {
#if !WITH_EDITOR

	SUBSCRIBE_METHOD(AFGWheeledVehicle::CalculateFuelNeed, [](auto& scope, const AFGWheeledVehicle* self)
		{
			//UE_LOG(FillTheTank_Log, Display, TEXT("CalculateFuelNeed"));
			scope.Override(self->GetMaxFuelEnergy());
		});
#endif
}


IMPLEMENT_GAME_MODULE(FFillTheTankModule, FillTheTank);