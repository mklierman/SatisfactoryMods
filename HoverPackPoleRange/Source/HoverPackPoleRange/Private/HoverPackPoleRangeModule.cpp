#include "HoverPackPoleRangeModule.h"
#include "FGPowerConnectionComponent.h"
#include "Equipment/FGHoverPack.h"
#include "Patching/NativeHookManager.h"
#include "FGCircuitConnectionComponent.h"
#include "FGPowerInfoComponent.h"
#include "Buildables/FGBuildablePowerPole.h"
#include "Equipment/FGEquipment.h"
#include "FGCharacterPlayer.h"
#include "HPPR_ConfigStruct.h"
#include "Util/RuntimeBlueprintFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Configuration/ConfigProperty.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "Configuration/Properties/ConfigPropertySection.h"
#include "FGGameMode.h"

DEFINE_LOG_CATEGORY(HoverPackPoleRange_Log);

void FHoverPackPoleRangeModule::StartupModule() {
#if !WITH_EDITOR
	AFGGameMode* LocalGameMode = GetMutableDefault<AFGGameMode>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGGameMode::PostLogin, LocalGameMode, [=](auto& scope, AFGGameMode* gm,
		APlayerController* pc)
		{
			if (gm->HasAuthority() && !gm->IsMainMenuGameMode())
			{
				SetConfigValues();
			}
		});
	//SetConfigValues();
	//	AFGHoverPack* CDOHoverPack = GetMutableDefault<AFGHoverPack>();

		//void AddHiddenConnection( class UFGCircuitConnectionComponent* other );
	SUBSCRIBE_METHOD(AFGHoverPack::OnPowerConnectionLocationUpdated, [=](auto& scope, AFGHoverPack* self, const FVector& NewLocation)
		{
			if (mAddedConnections.Contains(self) && mAddedConnections.Find(self))
			{
				return;
			}
			scope.Cancel();
		});


	SUBSCRIBE_METHOD(AFGHoverPack::DisconnectFromCurrentPowerConnection, [=](auto& scope, AFGHoverPack* self)
		{
			float distanceToCurrentConnection = self->GetDistanceFromCurrentConnection();
			float currentRange = self->mPowerConnectionSearchRadius;
			float remainingRange = currentRange - distanceToCurrentConnection;
			if (remainingRange <= 0)
			{
				return;
			}

			scope.Cancel();
		});

	SUBSCRIBE_METHOD(UFGCircuitConnectionComponent::AddHiddenConnection, [=](auto& scope, UFGCircuitConnectionComponent* self, class UFGCircuitConnectionComponent* other)
		{
			UE_LOG(HoverPackPoleRange_Log, Display, TEXT("AddHiddenConnection"));
			auto HoverPack = Cast<AFGHoverPack>(other->GetOwner());
			if (HoverPack)
			{
				FVector newConnectionLocation = self->GetOwner()->GetActorLocation();
				FVector currentConnectionLocation = HoverPack->GetCurrentConnectionLocation();
				FVector currentLocation = HoverPack->GetActorLocation();

				float distanceToCurrentConnection = HoverPack->GetDistanceFromCurrentConnection();
				float distanceToNewConnection = FVector::Distance(newConnectionLocation, currentConnectionLocation);

				float currentRange = HoverPack->mPowerConnectionSearchRadius;
				float newRange = mMk1Range * 100;

				FString newConnectionClass = self->GetOwner()->GetClass()->GetName();
				UE_LOG(HoverPackPoleRange_Log, Display, TEXT("newConnectionClass: %s"), *newConnectionClass);
				if (newConnectionClass == "Build_PowerPoleMk2_C" || newConnectionClass == "Build_PowerPoleWall_Mk2_C" || newConnectionClass == "Build_PowerPoleWallDouble_Mk2_C")
				{
					newRange = mMk2Range * 100;
				}
				else if (newConnectionClass == "Build_PowerPoleMk3_C" || newConnectionClass == "Build_PowerPoleWall_Mk3_C" || newConnectionClass == "Build_PowerPoleWallDouble_Mk3_C")
				{
					newRange = mMk3Range * 100;
				}

				float remainingRange = currentRange - distanceToCurrentConnection;

				UE_LOG(HoverPackPoleRange_Log, Display, TEXT("Current Range: %f, New Range: %f, Remaining Range %f"), currentRange, newRange, remainingRange);
				if (currentRange <= newRange)
				{
					UE_LOG(HoverPackPoleRange_Log, Display, TEXT("currentRange <= newRange"));
					self->RemoveHiddenConnection(other);
					mAddedConnections.Add(HoverPack, true);
					//mConnectionAdded = true;
					HoverPack->mPowerConnectionSearchRadius = newRange;
					return;
				}

				if (remainingRange > newRange)
				{
					UE_LOG(HoverPackPoleRange_Log, Display, TEXT("remainingRange > newRange"));
					//mConnectionAdded = false;
					mAddedConnections.Add(HoverPack, false);
					scope.Cancel();
				}
			}
			//if (mAllowAdd)
			//{
			//	return;
			//}
			//scope.Cancel();
		});

#endif
}

void FHoverPackPoleRangeModule::SetConfigValues()
{
	UE_LOG(HoverPackPoleRange_Log, Display, TEXT("SetConfigValues"));
	auto config = FHPPR_ConfigStruct::GetActiveConfig();
	mMk1Range = config.Mk1;
	mMk2Range = config.Mk2;
	mMk3Range = config.Mk3;
	mRailRange = config.Rails;
	mElseRange = config.EverythingElse;


	////auto config = Cast<UModConfiguration>(ModConfig);


	UConfigManager* ConfigManager = GEngine->GetEngineSubsystem<UConfigManager>();
	FConfigId ConfigId{ "HoverPackPoleRange", "" };
	auto Config = ConfigManager->GetConfigurationById(ConfigId);
	auto ConfigProperty = URuntimeBlueprintFunctionLibrary::GetModConfigurationPropertyByClass(Config);
	auto CPSection = Cast<UConfigPropertySection>(ConfigProperty);
	auto prop = CPSection->SectionProperties["Mk1"];
	FScriptDelegate Delegate;
	Delegate.BindUFunction(this, "Mk1Updated");
	prop->OnPropertyValueChanged.Add(Delegate);
}

void FHoverPackPoleRangeModule::Mk1Updated()
{
	UE_LOG(HoverPackPoleRange_Log, Display, TEXT("Mk1Updated"));
}

IMPLEMENT_GAME_MODULE(FHoverPackPoleRangeModule, HoverPackPoleRange);