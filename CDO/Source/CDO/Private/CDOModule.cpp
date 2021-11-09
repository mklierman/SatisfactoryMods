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

DEFINE_LOG_CATEGORY(CDO_Log);
void FCDOModule::StartupModule() {
	UFGHardDriveSettings* hdsCDO = GetMutableDefault<UFGHardDriveSettings>();
//#if !WITH_EDITOR
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

	SUBSCRIBE_METHOD(UFGItemDescriptor::GetStackSize, [](auto& scope, TSubclassOf<UFGItemDescriptor> item) {
		//UE_LOG(CDO_Log, Display, TEXT("GetStackSizeConverted"));
		if (item->IsValidLowLevel())
		{
			if (UFGItemDescriptor::GetForm(item) != EResourceForm::RF_LIQUID && item.GetDefaultObject()->mStackSize != EStackSize::SS_ONE)
			{
				//UE_LOG(CDO_Log, Display, TEXT("Override"));
				scope.Override(500);
			}
		}
		});

	SUBSCRIBE_METHOD(AFGCreature::GetIsArachnid, [](auto& scope, AFGCreature* self) {
		UE_LOG(CDO_Log, Display, TEXT("GetIsArachnid"));
		});

	SUBSCRIBE_METHOD(AFGCreatureSpawner::GetCreatureToSpawn, [](auto& scope, const AFGCreatureSpawner* self) {
		UE_LOG(CDO_Log, Display, TEXT("GetCreatureToSpawn: %s"), TCHAR_TO_UTF8(*self->mCreatureClass.GetDefaultObject()->GetName()));
		});
//#endif
}


IMPLEMENT_GAME_MODULE(FCDOModule, CDO);