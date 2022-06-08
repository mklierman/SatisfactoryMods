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
#include "Subsystem/SubsystemActorManager.h"
#include "HPPR_Subsystem.h"
#include "FGGameMode.h"

DEFINE_LOG_CATEGORY(HoverPackPoleRange_Log);


void FHoverPackPoleRangeModule::Loggit(FString myString)
{
	if (debugLogging)
	{
		UE_LOG(HoverPackPoleRange_Log, Display, TEXT("%s"), *myString);
	}
}

void FHoverPackPoleRangeModule::StartupModule() {
	//debugLogging = true;

#if !WITH_EDITOR
	AFGGameMode* LocalGameMode = GetMutableDefault<AFGGameMode>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGGameMode::PostLogin, LocalGameMode, [=](auto& scope, AFGGameMode* gm,
		APlayerController* pc)
		{
			if (gm->HasAuthority() && !gm->IsMainMenuGameMode())
			{
				UWorld* WorldObject = gm->GetWorld();
				USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
				mHPSubsystem = SubsystemActorManager->GetSubsystemActor<AHPPR_Subsystem>();
				mHPSubsystem->SetConfigValues();
			}
		});

	SUBSCRIBE_METHOD(AFGHoverPack::OnPowerConnectionLocationUpdated, [=](auto& scope, AFGHoverPack* self, const FVector& NewLocation)
		{
			if (mAddedConnections.Contains(self) && mAddedConnections[self] == true)
			{
				//Loggit("OnPowerConnectionLocationUpdated");
				//FString CurrentRailroadTrack = self->mCurrentRailroadTrack->GetName();
				//Loggit("mCurrentRailroadTrack: " + CurrentRailroadTrack);
				return;
			}
			scope.Cancel();
		});

	SUBSCRIBE_METHOD(AFGHoverPack::OnConnectionStatusUpdated, [=](auto& scope, AFGHoverPack* self, const bool HasConnection)
		{
			if (mAddedConnections.Contains(self) && mAddedConnections[self] == true)
			{
				//Loggit("OnConnectionStatusUpdated");
				return;
			}
			scope.Cancel();
		});


	SUBSCRIBE_METHOD(AFGHoverPack::DisconnectFromCurrentPowerConnection, [=](auto& scope, AFGHoverPack* self)
		{
			if ((mAllowRemove.Contains(self) && mAllowRemove[self] == true) || self->mPowerConnectionSearchTimer <= 0)
			{
				return;
			}
			float distanceToCurrentConnection = self->GetDistanceFromCurrentConnection();
			float currentRange = self->mPowerConnectionSearchRadius;
			float remainingRange = currentRange - distanceToCurrentConnection;
			if (remainingRange <= 0)
			{
				return;
			}
			//FString newConnectionClass = self->mCurrentPowerConnection->GetOwner()->GetClass()->GetName();
			//Loggit("Canceling DisconnectFromCurrentPowerConnection. Current mCurrentPowerConnection: " + newConnectionClass);
			scope.Cancel();
		});

	SUBSCRIBE_METHOD(UFGCircuitConnectionComponent::AddHiddenConnection, [=](auto& scope, UFGCircuitConnectionComponent* self, class UFGCircuitConnectionComponent* other)
		{
			auto HoverPack = Cast<AFGHoverPack>(other->GetOwner());
			if (HoverPack)
			{
				bool isRailConnection = false;
				FString newConnectionClass = self->GetOwner()->GetClass()->GetName();
				FVector newConnectionLocation = self->GetOwner()->GetActorLocation();
				if (newConnectionClass == "BP_RailroadSubsystem_C")
				{
					isRailConnection = true;

				}
				FVector currentConnectionLocation = HoverPack->GetCurrentConnectionLocation();
				FVector currentLocation = HoverPack->GetActorLocation();

				float distanceToCurrentConnection = HoverPack->GetDistanceFromCurrentConnection();
				float distanceToNewConnection = FVector::Distance(newConnectionLocation, currentConnectionLocation);
				float currentRange = HoverPack->mPowerConnectionSearchRadius;
				float newRange = mHPSubsystem->mMk1Range * 100;
				//FString newConnectionClass = self->GetOwner()->GetClass()->GetName();
				//Loggit("newConnectionClass: " + newConnectionClass);

				if (newConnectionClass == "Build_PowerPoleMk1_C" || newConnectionClass == "Build_PowerPoleWall_Mk1_C" || newConnectionClass == "Build_PowerPoleWallDouble_Mk1_C")
				{
					//Loggit("Mk1");
					newRange = mHPSubsystem->mMk1Range * 100;
				}
				else if (newConnectionClass == "Build_PowerPoleMk2_C" || newConnectionClass == "Build_PowerPoleWall_Mk2_C" || newConnectionClass == "Build_PowerPoleWallDouble_Mk2_C")
				{
					//Loggit("Mk2");
					newRange = mHPSubsystem->mMk2Range * 100;
				}
				else if (newConnectionClass == "Build_PowerPoleMk3_C" || newConnectionClass == "Build_PowerPoleWall_Mk3_C" || newConnectionClass == "Build_PowerPoleWallDouble_Mk3_C")
				{
					//Loggit("Mk3");
					newRange = mHPSubsystem->mMk3Range * 100;
				}
				else if (newConnectionClass == "BP_RailroadSubsystem_C")
				{
					//Loggit("Rail");
					newRange = mHPSubsystem->mRailRange * 100;
					isRailConnection = true;
				}
				else
				{
					//Loggit("Else");
					newRange = mHPSubsystem->mElseRange * 100;
				}

				//UE_LOG(HoverPackPoleRange_Log, Display, TEXT("distanceToCurrentConnection: %f"), distanceToCurrentConnection);
				float remainingRange = currentRange - distanceToCurrentConnection;

				//UE_LOG(HoverPackPoleRange_Log, Display, TEXT("Current Range: %f, New Range: %f, Remaining Range %f"), currentRange, newRange, remainingRange);
				if (remainingRange > newRange)
				{
					//Loggit("remainingRange > newRange");
					mAddedConnections.Add(HoverPack, false);
					mAllowRemove.Add(HoverPack, false);
					scope.Cancel();
				}
				else //if (remainingRange <= newRange)
				{
					mAllowRemove.Add(HoverPack, true);
					//Loggit("currentRange <= newRange");
					//self->RemoveHiddenConnection(other);
					//other->RemoveHiddenConnection(self);
					if (!isRailConnection)
					{
						HoverPack->DisconnectFromCurrentPowerConnection();
					}
					mAddedConnections.Add(HoverPack, true);
					HoverPack->mPowerConnectionSearchRadius = newRange;
					return;
				}
			}
		});

#endif
}

IMPLEMENT_GAME_MODULE(FHoverPackPoleRangeModule, HoverPackPoleRange);