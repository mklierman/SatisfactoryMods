#include "UniversalVehiclePathsModule.h"
#include "Patching/NativeHookManager.h"
#include "FGVehicleSubsystem.h"
#include "WheeledVehicles/FGWheeledVehicle.h"

DEFINE_LOG_CATEGORY(UniversalVehiclePaths_Log);
void FUniversalVehiclePathsModule::StartupModule() {
#if !WITH_EDITOR
	SUBSCRIBE_METHOD(AFGVehicleSubsystem::FindSavedWheeledVehiclePaths, [](auto& scope, AFGVehicleSubsystem* self, const FString& textFilter, TSubclassOf< class AFGWheeledVehicle > typeFilter, const AFGWheeledVehicle* vehicle, TArray< AFGSavedWheeledVehiclePath* >& result)
		{
			//UE_LOG(UniversalVehiclePaths_Log, Display, TEXT("FindSavedWheeledVehiclePaths"));
			if (vehicle->GetClass()->GetSuperClass()->GetName().Contains("6W"))
			{
				scope(self, textFilter, vehicle->GetClass()->GetSuperClass()->GetSuperClass(), vehicle, result);
				return;
			}
			else
			{
				scope(self, textFilter, vehicle->GetClass()->GetSuperClass(), vehicle, result);
			}
			
		});
#endif
}


IMPLEMENT_GAME_MODULE(FUniversalVehiclePathsModule, UniversalVehiclePaths);