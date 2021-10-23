#include "HoverPackPoleRangeModule.h"
#include "FGPowerConnectionComponent.h"
#include "Equipment/FGHoverPack.h"
#include "Patching/NativeHookManager.h"
#include "FGCircuitConnectionComponent.h"
#include "FGPowerInfoComponent.h"
#include "Buildables/FGBuildablePowerPole.h"
#include "Equipment/FGEquipment.h"
#include "FGCharacterPlayer.h"
#include "Util/RuntimeBlueprintFunctionLibrary.h"

DEFINE_LOG_CATEGORY(HoverPackPoleRange_Log);
void FHoverPackPoleRangeModule::StartupModule() {
	AFGHoverPack* CDOHoverPack = GetMutableDefault<AFGHoverPack>();
//#if !WITH_EDITOR
//	UE_LOG(HoverPackPoleRange_Log, Display, TEXT("Starting module"));
//	SUBSCRIBE_METHOD(AFGHoverPack::ConnectToPowerConnection, [](auto& scope, AFGHoverPack* self, class UFGPowerConnectionComponent* Connection) {
//		UE_LOG(HoverPackPoleRange_Log, Display, TEXT("Starting Hook"));
//
//		UE_LOG(HoverPackPoleRange_Log, Display, TEXT("end"));
//
//		});
//
//#endif
//	//SUBSCRIBE_METHOD(AFGHoverPack::IsPowerConnectionValid, [](auto& scope,const AFGHoverPack* self, UFGPowerConnectionComponent* Connection, bool CheckDistance = true) {
//	//	UE_LOG(HoverPackPoleRange_Log, Display, TEXT("IsPowerConnectionValid1"));
//	//	scope.Override(false);
//	//	});
//
//	//SUBSCRIBE_METHOD(AFGHoverPack::IsPowerConnectionValid, [](auto& scope, const AFGHoverPack* self, UFGPowerConnectionComponent* Connection, bool CheckDistance) {
//	//	UE_LOG(HoverPackPoleRange_Log, Display, TEXT("IsPowerConnectionValid2"));
//	//	scope.Override(false);
//	//	});
//
//	//SUBSCRIBE_METHOD(AFGHoverPack::IsPowerConnectionValid, [](auto& scope, const AFGHoverPack* self, class UFGPowerConnectionComponent* Connection, bool CheckDistance = true) {
//	//	UE_LOG(HoverPackPoleRange_Log, Display, TEXT("IsPowerConnectionValid3"));
//	//	scope.Override(false);
//	//	});
//
//	SUBSCRIBE_METHOD(AFGHoverPack::IsPowerConnectionValid, [](auto& scope, const AFGHoverPack* self, class UFGPowerConnectionComponent* Connection, bool CheckDistance = true) {
//		UE_LOG(HoverPackPoleRange_Log, Display, TEXT("IsPowerConnectionValid4"));
//		scope.Override(false);
//		});
//
//	//ConnectToNearestPowerConnection
//	SUBSCRIBE_METHOD(AFGHoverPack::ConnectToNearestPowerConnection, [](auto& scope, const AFGHoverPack* self)
//		{
//			UE_LOG(HoverPackPoleRange_Log, Display, TEXT("ConnectToNearestPowerConnection"));
//		});
//
//	SUBSCRIBE_METHOD(AFGHoverPack::GetPowerConnectionStatus, [](auto& scope, const AFGHoverPack* self, class UFGPowerConnectionComponent* Connection)
//		{
//			TArray<AActor*> children;
//			self->GetAllChildActors(children, true);
//			for (AActor* child : children)
//			{
//				FString childName = child->GetName();
//				UE_LOG(HoverPackPoleRange_Log, Display, TEXT("%s"), *FString(childName));
//			}
//			return;
//			if (Connection == nullptr || !Connection->mHasPower || self->GetCurrentConnection() == nullptr)
//			{
//				return;
//			}
//			//auto bleh = self->IsPowerConnectionValid(Connection);
//			scope.Override(EHoverConnectionStatus::HCS_NO_POWER);
//			return;
//			//UE_LOG(HoverPackPoleRange_Log, Display, TEXT("GetPowerConnectionStatus"));
//			FConfigId configID;
//			configID.ModReference = "HoverPackPoleRange";
//			UConfigProperty* config = URuntimeBlueprintFunctionLibrary::Conv_ConfigIdToConfigProperty(configID);
//			UConfigPropertySection* configSection = Cast<UConfigPropertySection>(config);
//			UConfigProperty* mk2Prop = URuntimeBlueprintFunctionLibrary::Conv_ConfigPropertySectionToConfigProperty(configSection, "Mk2");
//			UConfigProperty* mk3Prop = URuntimeBlueprintFunctionLibrary::Conv_ConfigPropertySectionToConfigProperty(configSection, "Mk3");
//			float mk2Mult = URuntimeBlueprintFunctionLibrary::Conv_ConfigPropertyToFloat(mk2Prop);
//			float mk3Mult = URuntimeBlueprintFunctionLibrary::Conv_ConfigPropertyToFloat(mk3Prop);
//			float mk1Range = 3200.0;
//			float mk2Range = mk1Range * mk2Mult;
//			float mk3Range = mk1Range * mk3Mult;
//
//			//UE_LOG(HoverPackPoleRange_Log, Display, TEXT("Mk1 Range: %f, Mk2 Range: %f, Mk3Range %f"), mk1Range, mk2Range, mk3Range);
//
//			FVector newConnectionLocation;
//			newConnectionLocation = Connection->GetOwner()->GetActorLocation();
//			FVector currentConnectionLocation = self->GetCurrentConnectionLocation();
//			FVector currentLocation = self->GetOwner()->GetActorLocation();
//
//			float distanceToCurrentConnection = self->GetDistanceFromCurrentConnection();
//			float distanceToNewConnection = FVector::Distance(newConnectionLocation, currentConnectionLocation);
//
//			float currentRange = mk1Range;
//			float newRange = mk1Range;
//
//			FString newConnectionClass = Connection->GetOwner()->GetClass()->GetName();
//			FString currentConnectionClass = self->GetCurrentConnection()->GetOwner()->GetClass()->GetName();
//			//UE_LOG(HoverPackPoleRange_Log, Display, TEXT("New Connection Class : %s"), *FString(newConnectionClass));
//			if (newConnectionClass == "Build_PowerPoleMk2_C" || newConnectionClass == "Build_PowerPoleWall_Mk2_C" || newConnectionClass == "Build_PowerPoleWallDouble_Mk2_C")
//			{
//				newRange = mk2Range;
//			}
//			else if (newConnectionClass == "Build_PowerPoleMk3_C" || newConnectionClass == "Build_PowerPoleWall_Mk3_C" || newConnectionClass == "Build_PowerPoleWallDouble_Mk3_C")
//			{
//				newRange = mk3Range;
//			}
//
//			if (currentConnectionClass == "Build_PowerPoleMk2_C" || currentConnectionClass == "Build_PowerPoleWall_Mk2_C" || currentConnectionClass == "Build_PowerPoleWallDouble_Mk2_C")
//			{
//				currentRange = mk2Range;
//			}
//			else if (currentConnectionClass == "Build_PowerPoleMk3_C" || currentConnectionClass == "Build_PowerPoleWall_Mk3_C" || currentConnectionClass == "Build_PowerPoleWallDouble_Mk3_C")
//			{
//				currentRange = mk3Range;
//			}
//
//			float remainingRange = currentRange - distanceToCurrentConnection;
//
//			UE_LOG(HoverPackPoleRange_Log, Display, TEXT("Current Range: %f, New Range: %f, Remainging Range %f"), currentRange, newRange, remainingRange);
//			if (currentRange <= newRange)
//			{
//				UE_LOG(HoverPackPoleRange_Log, Display, TEXT("currentRange <= newRange"));
//				return;
//			}
//
//			if (remainingRange > newRange)
//			{
//				UE_LOG(HoverPackPoleRange_Log, Display, TEXT("remainingRange > newRange"));
//				scope.Override(EHoverConnectionStatus::HCS_OUT_OF_RANGE);
//				return;
//				//scope.Cancel();
//			}
//
//			UE_LOG(HoverPackPoleRange_Log, Display, TEXT("Something else?"));
//			//scope.Override(EHoverConnectionStatus::HCS_OUT_OF_RANGE);
//		});
}


IMPLEMENT_GAME_MODULE(FHoverPackPoleRangeModule, HoverPackPoleRange);