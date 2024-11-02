#include "HoverPackPoleRangeModule.h"

DEFINE_LOG_CATEGORY(HoverPackPoleRange_Log);

#pragma optimize("", off)

void FHoverPackPoleRangeModule::Loggit(FString myString)
{
	if (debugLogging)
	{
		UE_LOG(HoverPackPoleRange_Log, Display, TEXT("%s"), *myString);
	}
}

void FHoverPackPoleRangeModule::FindNearestConnection(AFGHoverPack* self)
{
	UClass* pcc = UFGPowerConnectionComponent::StaticClass();
	TArray< UFGPowerConnectionComponent*> PossibleConnectionComponents;

	// Trace multi for static in a sphere based on range
	//FCollisionShape MySphere = FCollisionShape::MakeSphere(500.0f); // 5M Radius
	//ECollisionChannel TraceChannel = ECollisionChannel::ECC_WorldStatic;
	//self->GetWorld()->SweepMultiByChannel(OutResults, self->GetActorLocation() , self->GetActorLocation(), FQuat::Identity, TraceChannel, MySphere);


	TArray<FHitResult> OutResults;
	TArray<AActor*> OutActors;
	TArray<UPrimitiveComponent*> OutComponents;
	TArray<AActor*> ActorsToIgnore;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesArray;
	ObjectTypesArray.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
	bool traceHit = UKismetSystemLibrary::SphereTraceMultiForObjects(self->GetWorld(), self->GetActorLocation(), self->GetActorLocation(), 500.0f, ObjectTypesArray, false, ActorsToIgnore, EDrawDebugTrace::None, OutResults, true);
	
	// Pick out power connection components, making sure to go through abstract instance managers
	if (traceHit)
	{
		for (auto hit : OutResults)
		{
			if (hit.GetActor())
			{
				auto hitBuildable = Cast<AFGBuildable>(hit.GetActor());
				if (hitBuildable)
				{
					UFGPowerConnectionComponent* comp = GetBuildablePowerConnectionComponent(hitBuildable);
					if (comp)
					{
						PossibleConnectionComponents.Add(comp);
						continue;
					}
				}

				auto hitInstanceMgr = Cast<AAbstractInstanceManager>(hit.GetActor());
				if (hitInstanceMgr)
				{
					FInstanceHandle outHandle;
					auto hitResolved = hitInstanceMgr->ResolveHit(hit, outHandle);
					if (hitResolved)
					{
						if (outHandle.GetOwner())
						{
							auto handleBuildable = Cast<AFGBuildable>(outHandle.GetOwner());
							if (handleBuildable)
							{
								UFGPowerConnectionComponent* comp2 = GetBuildablePowerConnectionComponent(handleBuildable);
								if (comp2)
								{
									PossibleConnectionComponents.Add(comp2);
								}
							}
						}
					}
				}

				auto hitRail = Cast<AFGBuildableRailroadTrack>(hit.GetActor());
				if (hitRail)
				{
					auto railPower = hitRail->GetThirdRail();
					if (railPower)
					{
						PossibleConnectionComponents.Add(railPower);
					}
				}
			}
		}

		for (auto possibility : PossibleConnectionComponents)
		{
			auto owner = possibility->GetOwner();
		}
	}


	// Trace multi for static in a sphere based on range

	// Pick out power connection components, making sure to go through abstract instance managers
	
	// Filter out connections that aren't powered

	// Sort connections by type (mk1, mk2, etc)

	// Starting with highest range type, get distances to connections

	// If any of the found connections have a larger radius than our current remaining radius, store it and break out

	// If not, continue searching through remaining types

	// Do the same for rails

	// If there is a found connection that is better than our current one, add hidden connection to it
}

UFGPowerConnectionComponent* FHoverPackPoleRangeModule::GetBuildablePowerConnectionComponent(AFGBuildable* hitBuildable)
{
	if (hitBuildable)
	{
		auto components = hitBuildable->GetComponents();
		if (components.Num() > 0)
		{
			for (auto component : components)
			{
				UFGPowerConnectionComponent* pccomp = Cast< UFGPowerConnectionComponent>(component);
				if (pccomp && pccomp->HasPower())
				{
					return pccomp; //We don't care if there are other powered components since they will be at the same location
				}
			}
		}
	}
	return nullptr;
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
	//AFGHoverPack::ConnectToNearestPowerConnection()
	SUBSCRIBE_METHOD(AFGHoverPack::ConnectToNearestPowerConnection, [=](auto& scope, AFGHoverPack* self)
		{
			FindNearestConnection(self);
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
#pragma optimize("", on)

IMPLEMENT_GAME_MODULE(FHoverPackPoleRangeModule, HoverPackPoleRange);