#include "UniversalVehiclePathsModule.h"
#include "Patching/NativeHookManager.h"

DEFINE_LOG_CATEGORY(UniversalVehiclePaths_Log);

#pragma optimize("", off)
void FUniversalVehiclePathsModule::StartupModule() {
#if !WITH_EDITOR
	SUBSCRIBE_METHOD(AFGVehicleSubsystem::FindSavedWheeledVehiclePaths, [this](auto& scope, AFGVehicleSubsystem* self, const FString& textFilter, TSubclassOf< class AFGWheeledVehicle > typeFilter, const AFGWheeledVehicleInfo* vehicle, TArray< AFGSavedWheeledVehiclePath* >& result)
		{
			if (!self || !vehicle)
			{
				return;
			}
			//UE_LOG(UniversalVehiclePaths_Log, Display, TEXT("FindSavedWheeledVehiclePaths"));
			if (!vehicle->GetVehicle() && vehicle->mReplicatedVehicle)
			{

				if (vehicle->mReplicatedVehicle->GetClass()->GetSuperClass()->GetName().Contains("6W"))
				{
					scope(self, textFilter, vehicle->GetVehicle()->GetClass()->GetSuperClass()->GetSuperClass(), vehicle, result);
				}
				else
				{
					scope(self, textFilter, vehicle->mReplicatedVehicle->GetClass()->GetSuperClass(), vehicle, result);
				}
				return;
			}
			if (vehicle->GetVehicle()->GetClass()->GetSuperClass()->GetName().Contains("6W"))
			{
				scope(self, textFilter, vehicle->GetVehicle()->GetClass()->GetSuperClass()->GetSuperClass(), vehicle, result);
			}
			else
			{
				scope(self, textFilter, vehicle->GetVehicle()->GetClass()->GetSuperClass(), vehicle, result);
			}

		});
#endif
}

void FUniversalVehiclePathsModule::FindSavedPaths(AFGVehicleSubsystem* self, const AFGWheeledVehicleInfo* vehicle)
{
	auto newSelf = self;
	auto newVehicle = vehicle;
}

#pragma optimize("", on)

IMPLEMENT_GAME_MODULE(FUniversalVehiclePathsModule, UniversalVehiclePaths);