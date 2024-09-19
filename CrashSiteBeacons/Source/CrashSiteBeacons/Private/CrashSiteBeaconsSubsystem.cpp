#include "CrashSiteBeaconsSubSystem.h"
#include "FGDropPod.h"
#include "Patching/NativeHookManager.h"
#include "Patching/BlueprintHookManager.h"
#include "Patching/BlueprintHookHelper.h"
#include "FGMapManager.h"
#include "FGMapMarker.h"
#include "FGIconLibrary.h"
#include "FGActorRepresentationManager.h"
#include "Representation/FGMapMarkerRepresentation.h"
#include "FGActorRepresentation.h"

#pragma optimize("", off)

DEFINE_LOG_CATEGORY(CrashSiteBeacons_Log);


template<typename T>
int AddValueToEnum(FName NewName)
{
	UEnum* Enum = StaticEnum<T>();
	FString EnumName = Enum->GetName();
	if (!NewName.ToString().StartsWith(EnumName))
	{
		NewName = FName(FString::Printf(TEXT("%s::%s"), *EnumName, *NewName.ToString()));
	}
	if (Enum->IsValidEnumName(NewName))
	{
		return -1;
	}
	bool HasMax = Enum->ContainsExistingMax();
	int64 NewValue = HasMax ? Enum->GetMaxEnumValue() : Enum->NumEnums();
	TArray<TPair<FName, int64>> Names;
	int EnumNum = Enum->NumEnums();
	if (HasMax)
	{
		EnumNum--;
	}
	for (int i = 0; i < EnumNum; ++i)
	{
		FName Name = Enum->GetNameByIndex(i);
		int64 Value = Enum->GetValueByIndex(i);
		Names.Emplace(TPair<FName, int64>(Name, Value));
		FString enumName = Enum->GetNameStringByIndex(i);
		FString enumIdx = FString::FromInt(Value);
		//UE_LOG(CrashSiteBeacons_Log, Display, TEXT("Enum Name/Value: %s/%s"), *enumName, *enumIdx);
	}
	auto New = TPair<FName, int64>(NewName, NewValue);
	Names.Emplace(New);
	EEnumFlags Flag = Enum->HasAnyEnumFlags(EEnumFlags::Flags) ? EEnumFlags::Flags : EEnumFlags::None;
	Enum->SetEnums(Names, Enum->GetCppForm(), Flag, HasMax);
	return EnumNum;
}

void ACrashSiteBeaconsSubSystem::GenerateMapMarker(FString markerName, FVector_NetQuantize markerLocation)
{
	AddEnumType();

	UEnum* Enum = StaticEnum<ERepresentationType>();
	auto mapManager = AFGMapManager::Get(this);
	EnumIndex = Enum->GetValueByNameString("RT_CrashSite");
	FString enumName = Enum->GetNameStringByIndex(EnumIndex);
	FString enumIdx = FString::FromInt(EnumIndex);
	//UE_LOG(CrashSiteBeacons_Log, Display, TEXT("Enum Name/Index: %s/%s"), *enumName, *enumIdx);
	FMapMarker marker;
	marker.CompassViewDistance = ECompassViewDistance::CVD_Off;
	marker.Location = markerLocation;
	marker.Name = markerName;
	marker.Color = FLinearColor::Red;
	marker.IconID = 652;
	marker.MapMarkerType = static_cast<ERepresentationType>(EnumIndex);
	mapManager->AddNewMapMarker(marker, marker);
}

void ACrashSiteBeaconsSubSystem::ForceRemoveMapMarker(const FMapMarker &marker)
{
	auto mapManager = AFGMapManager::Get(this);
	auto repManager = AFGActorRepresentationManager::Get(this);

	TArray<FMapMarker> markersToRemove;
	auto rep = mapManager->FindMapMarkerRepresentation(marker);
	if (rep)
	{
		if (auto actorRep = Cast<UFGActorRepresentation>(rep))
		{
			repManager->RemoveRepresentation(actorRep);
		}
	}
	//for (FMapMarker mkr : mapManager->mMapMarkers)
	//{
	//	if (mkr.Color == marker.Color && mkr.Name == marker.Name && mkr.Location == marker.Location)
	//	{
	//		mkr.MarkerID = 255;
	//		mkr.Location = FVector(0, 0, 0);
	//		mkr.Name = "";
	//		mkr.MapMarkerType = ERepresentationType::RT_Default;
	//		mkr.IconID = 0;
	//		mkr.Color= FLinearColor::Black;
	//		mkr.Scale= 1.0;
	//		mkr.CompassViewDistance = ECompassViewDistance::CVD_Off;
	//		mkr.CreatedByLocalPlayer = false;
	//		markersToRemove.Add(mkr);
	//	}
	//}
	//mapManager->mMapMarkers.Remove(marker);
}

FVector ACrashSiteBeaconsSubSystem::GetMapMarkerRepresentationLocation(const FMapMarker& mapMarker)
{
	auto mapManager = AFGMapManager::Get(this);
	auto repManager = AFGActorRepresentationManager::Get(this);
	auto rep = mapManager->FindMapMarkerRepresentation(mapMarker);
	if (rep)
	{
		return rep->GetActorLocation();
	}

	return FVector();
}

void ACrashSiteBeaconsSubSystem::RemoveRepresentation(UFGActorRepresentation* actorRepresentation)
{
	auto mapManager = AFGMapManager::Get(this);
	auto repManager = AFGActorRepresentationManager::Get(this);

	TArray<UFGActorRepresentation*>ToRemove;
	TArray<FMapMarker> markersToRemove;
	for (UFGActorRepresentation* rep : repManager->mAllRepresentations)
	{
		if (rep == actorRepresentation)
		{
			ToRemove.Add(rep);
			
		}
	}
	for (auto rep : ToRemove)
	{
		repManager->mAllRepresentations.Remove(rep);
	}


	//repManager->mReplicatedRepresentations.Remove(actorRepresentation);
	//repManager->mAllRepresentations.Remove(actorRepresentation);
	//repManager->RemoveRepresentation(actorRepresentation);
}

void ACrashSiteBeaconsSubSystem::UpdateRepresentation(UFGActorRepresentation* actorRepresentation)
{
	actorRepresentation->mRepresentationType = ERepresentationType::RT_StartingPod;
}

void ACrashSiteBeaconsSubSystem::AddEnumType()
{
		UEnum* Enum = StaticEnum<ERepresentationType>();
		if (Enum->GetValueByNameString("RT_CrashSite") == INDEX_NONE)
		{
			int newIndex = Enum->NumEnums() - 1;
			AddValueToEnum<ERepresentationType>(FName("RT_CrashSite"));
			//for (int i = 0; i < newIndex + 1; i++)
			//{
			//	FString enumName = Enum->GetNameStringByIndex(i);
			//	FString enumIdx = FString::FromInt(i);
			//	//UE_LOG(CrashSiteBeacons_Log, Display, TEXT("Enum Name/Index: %s/%s"), *enumName, *enumIdx);
			//}
			if (newIndex > -1)
			{
				EnumIndex = newIndex;
				FString enumIdx = FString::FromInt(newIndex);
				//UE_LOG(CrashSiteBeacons_Log, Display, TEXT("newIndex: %s"), *enumIdx);
			}
		}
		auto mapManager = AFGMapManager::Get(this);
		mapManager->mMapMarkers.Empty();
}

void OnDropPodOpen(CallScope<void(*)(AFGDropPod*)>& scope, AFGDropPod* DropPod)
{
	auto mapManager = AFGMapManager::Get(DropPod->GetWorld());
	TArray<FMapMarker> markers;
	mapManager->GetMapMarkers(markers);
	if (markers.Num() > 0)
	{
		FVector podLoc = DropPod->GetActorLocation();
		for (auto marker : markers)
		{
			if (marker.Location.X == podLoc.X && marker.Location.Y == podLoc.Y && marker.Location.Z == podLoc.Z
				&& marker.MapMarkerType == ERepresentationType::RT_StartingPod)
			{
				mapManager->RemoveMapMarker(marker);
			}
		}
	}
}


void ACrashSiteBeaconsSubSystem::HookCrashSites()
{
#if !WITH_EDITOR
	//SUBSCRIBE_METHOD(AFGDropPod::Open, &OnDropPodOpen);
#endif
}

bool ACrashSiteBeaconsSubSystem::HasPodBeenLooted(AFGDropPod* pod)
{
	return pod->HasBeenLooted();
}

int ACrashSiteBeaconsSubSystem::GetNewEnumValue()
{
	UEnum* Enum = StaticEnum<ERepresentationType>();
	return Enum->GetValueByNameString("ERepresentationType::RT_CrashSite");
}

void ACrashSiteBeaconsSubSystem::ClearAllMarkers()
{
	auto repManager = AFGActorRepresentationManager::Get(this);
	auto mapManager = AFGMapManager::Get(this);

	mapManager->mMapMarkers.Empty();
	mapManager->mMapMarkers.Reset();
	int deletedNum = 0;
	int savedNum = 0;
	TArray<UFGActorRepresentation*>ToRemove;
	for (UFGActorRepresentation* rep : repManager->mAllRepresentations)
	{
		if (rep->GetRepresentationType() != ERepresentationType::RT_Default
			&& rep->GetRepresentationType() != ERepresentationType::RT_Beacon
			&& rep->GetRepresentationType() != ERepresentationType::RT_Crate
			&& rep->GetRepresentationType() != ERepresentationType::RT_Hub
			&& rep->GetRepresentationType() != ERepresentationType::RT_Ping
			&& rep->GetRepresentationType() != ERepresentationType::RT_Player
			&& rep->GetRepresentationType() != ERepresentationType::RT_RadarTower
			&& rep->GetRepresentationType() != ERepresentationType::RT_Resource
			&& rep->GetRepresentationType() != ERepresentationType::RT_SpaceElevator
			&& rep->GetRepresentationType() != ERepresentationType::RT_StartingPod
			&& rep->GetRepresentationType() != ERepresentationType::RT_Train
			&& rep->GetRepresentationType() != ERepresentationType::RT_TrainStation
			&& rep->GetRepresentationType() != ERepresentationType::RT_Vehicle
			&& rep->GetRepresentationType() != ERepresentationType::RT_VehicleDockingStation
			&& rep->GetRepresentationType() != ERepresentationType::RT_DronePort
			&& rep->GetRepresentationType() != ERepresentationType::RT_Drone)
		{
			ToRemove.Add(rep);
			//repManager->mAllRepresentations.Remove(rep);
			deletedNum++;
		}
		else
		{
			savedNum++;
		}
	}
	for (UFGActorRepresentation* rep : ToRemove)
	{
		repManager->mAllRepresentations.Remove(rep);
	}
}

#pragma optimize("", on)