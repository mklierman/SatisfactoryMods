#include "CDOModule.h"
#include "Hologram/FGFactoryBuildingHologram.h"
#include "FGClearanceComponent.h"
#include "Patching/NativeHookManager.h"
#include "Creature/FGCreature.h"
#include "Creature/Enemy/FGEnemy.h"
#include "Creature/FGCreatureSpawner.h"
#include "FGHardDriveSettings.h"
#include "Resources/FGItemDescriptor.h"
#include "WheeledVehicles/FGWheeledVehicle.h"
#include "WheeledVehicles/FGWheeledVehicleMovementComponent6W.h"
#include "WheeledVehicles/FGWheeledVehicleMovementComponent4W.h"
#include "WheeledVehicleMovementComponent.h"
#include "FGResourceSinkSettings.h"
#include "Equipment/FGResourceScanner.h"
#include "Buildables/FGBuildableWidgetSign.h"
#include "Hologram/FGConveyorBeltHologram.h"
#include "Hologram/FGPoleHologram.h"
#include "Hologram/FGBuildableHologram.h"
#include "FGDroneVehicle.h"

DEFINE_LOG_CATEGORY(CDO_Log);


//bool MSPlacementThing(T scope, AFGPoleHologram* self, bool isInputFromARelease)
//{
//	return false;
//}

void FCDOModule::StartupModule() {

#if !WITH_EDITOR
	AFGDroneVehicle* droneCDO = GetMutableDefault<AFGDroneVehicle>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGDroneVehicle::IsUseable_Implementation, droneCDO, [](auto scope, const AFGDroneVehicle* self)
		{
			scope.Override(true);
		});
#endif
	//
	//UFGHardDriveSettings* hdsCDO = GetMutableDefault<UFGHardDriveSettings>();
	//hdsCDO->mUniqueItemCount = 6;
	//SUBSCRIBE_METHOD_VIRTUAL(AFGCreatureSpawner::BeginPlay, csCDO, [](auto scope, AFGCreatureSpawner* self)
	//	{
	//		FStringClassReference HogClassReference = FStringClassReference(TEXT("/Game/FactoryGame/Character/Creature/Enemy/Hog/Char_Hog.Char_Hog"));
	//		if (UClass* RecipeClass = HogClassReference.TryLoadClass<AFGEnemy>())
	//		{
	//			self->mCreatureClass = HogClassReference.TryLoadClass<AFGEnemy>()->GetClass();
	//		}
	//		//if (self->mCreatureClass->GetName().Contains("stinger"))
	//		//{
	//
	//			//self->mCreatureClass = HogClassReference.TryLoadClass<AFGEnemy>();
	//		//}

	//	});
	// UFUNCTION( BlueprintCallable, Category = "Vehicle" )
	//void SetMovementComponent(UWheeledVehicleMovementComponent * movementComponent);
	//UFGWheeledVehicleMovementComponent4W* fourWCDO = GetMutableDefault<UFGWheeledVehicleMovementComponent4W>();
	//SUBSCRIBE_METHOD_VIRTUAL(UFGWheeledVehicleMovementComponent4W::GenerateTireForces, fourWCDO, [](auto& scope, UFGWheeledVehicleMovementComponent4W* self, class UVehicleWheel* Wheel, const FTireShaderInput& Input, FTireShaderOutput& Output) {

	//	});
	// 	void PrecacheObjects();
	//AFGResourceScanner* rsCDO = GetMutableDefault<AFGResourceScanner>();
	//SUBSCRIBE_METHOD_VIRTUAL(AFGResourceScanner::OnScanPressed, rsCDO, [](auto& scope, AFGResourceScanner* self) {
	//	//self->mNodeClusters.Reset();
	//	 self->GenerateNodeClusters();
	// });
	//SUBSCRIBE_METHOD(AFGWheeledVehicle::SetMovementComponent, [](auto& scope, AFGWheeledVehicle* self, UWheeledVehicleMovementComponent* movementComponent) {
		//self->mMaxAssistedAcceleration = self->mMaxAssistedAcceleration * 10;
		/*UFGWheeledVehicleMovementComponent4W* four = Cast< UFGWheeledVehicleMovementComponent4W>(movementComponent);
		if (four)
		{
			UE_LOG(CDO_Log, Display, TEXT("four"));
			UE_LOG(CDO_Log, Display, TEXT("mDSOLMaxSlopeAngleLimit = %f"), four->mDSOLMaxSlopeAngleLimit);
			UE_LOG(CDO_Log, Display, TEXT("mSlopeShiftRatio = %f"), four->mSlopeShiftRatio);
			UE_LOG(CDO_Log, Display, TEXT("MaxEngineRPM = %f"), four->MaxEngineRPM);
			four->MaxEngineRPM = four->MaxEngineRPM * 10;
			UE_LOG(CDO_Log, Display, TEXT("MaxEngineRPM = %f"), four->MaxEngineRPM);
		}
		UFGWheeledVehicleMovementComponent6W* six = Cast< UFGWheeledVehicleMovementComponent6W>(movementComponent);
		if (six)
		{
			UE_LOG(CDO_Log, Display, TEXT("mDSOLMaxSlopeAngleLimit = %f"), six->mDSOLMaxSlopeAngleLimit);
			UE_LOG(CDO_Log, Display, TEXT("mSlopeShiftRatio = %f"), six->mSlopeShiftRatio);
			UE_LOG(CDO_Log, Display, TEXT("mDownShiftLatency = %f"), six->mDownShiftLatency);
			UE_LOG(CDO_Log, Display, TEXT("mUseDSOLGearbox = %s"), six->mUseDSOLGearbox ? "true" : "false");
			six->mSlopeShiftRatio = 1;
		}*/
		//});

//#endif

}



IMPLEMENT_GAME_MODULE(FCDOModule, CDO);