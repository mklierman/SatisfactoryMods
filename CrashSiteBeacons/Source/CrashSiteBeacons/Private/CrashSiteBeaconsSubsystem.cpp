#include "CrashSiteBeaconsSubSystem.h"
#include "FGDropPod.h"
#include "Patching/NativeHookManager.h"
#include "Patching/BlueprintHookManager.h"
#include "Patching/BlueprintHookHelper.h"
#include "FGMapManager.h"
#include "FGMapMarker.h"
#include "FGIconLibrary.h"
#include "FGActorRepresentation.h"


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
		UE_LOG(CrashSiteBeacons_Log, Display, TEXT("Enum Name/Value: %s/%s"), *enumName, *enumIdx);
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
			if (marker.Location.X == podLoc.X && marker.Location.Y == podLoc.Y && marker.Location.Z == podLoc.Z)
			{
				mapManager->RemoveMapMarker(marker);
			}
		}
	}
}


void ACrashSiteBeaconsSubSystem::HookCrashSites()
{
#if !WITH_EDITOR
	SUBSCRIBE_METHOD(AFGDropPod::Open, &OnDropPodOpen)
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
