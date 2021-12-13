// Copyright Epic Games, Inc. All Rights Reserved.

#include "StationNames.h"
#include "Patching/NativeHookManager.h"
#include "Buildables/FGBuildableDockingStation.h"
#include "FGMapAreaTexture.h"
#include "FGMapArea.h"
#include "FGInventoryComponent.h"
#include "FGPlayerController.h"
#include "FGBlueprintFunctionLibrary.h"


DEFINE_LOG_CATEGORY(StationNames_Log);
void FStationNamesModule::StartupModule()
{
#if !WITH_EDITOR


	AFGBuildableDockingStation* CDODockingStation = GetMutableDefault<AFGBuildableDockingStation>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableDockingStation::Tick, CDODockingStation, [](AFGBuildableDockingStation* self, float dseconds) {


		FText areaText = FText::FromString("Empty");
		AFGPlayerController* player = UFGBlueprintFunctionLibrary::GetLocalPlayerController(self->GetWorld());
		if (player)
		{
			UFGMapAreaTexture* mat = player->GetMapAreaTexture(); if (mat)
			{
				//UFGMapAreaTexture* mat = UFGMapAreaTexture::StaticClass();
				auto mapArea = mat->GetMapAreaForWorldLocation(self->GetActorLocation());
				if (mapArea)
				{
					areaText = UFGMapArea::GetAreaDisplayName(mapArea);
				}
			}
		}

		FString loadModeString = self->GetIsInLoadMode() ? "Load" : "Unload";
		FText loadModeText = FText::FromString(loadModeString);
		auto firstItemIndex = self->GetInventory()->GetFirstIndexWithItem();
		FText inventoryText = FText::FromString("Empty");
		if (firstItemIndex > 0)
		{
			auto invComp = self->GetInventory();
			FInventoryStack invStack = invComp->GetStackFromIndex(firstItemIndex);
			FString inventoryString = invStack.Item.ItemClass.Get()->GetName();
			inventoryText = FText::FromString(inventoryString);
		}

		UE_LOG(StationNames_Log, Display, TEXT("Map Area: %s | Load Mode: %s | Item: %s"), *areaText.ToString(), *loadModeText.ToString(), *inventoryText.ToString());
		});

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableDockingStation::BeginPlay, CDODockingStation, [](AFGBuildableDockingStation* self) {
		struct NameStruct
		{
			FString Name;
			bool CanLink;
		};
		TArray<NameStruct> Name1;
		Name1.Add(NameStruct{ "Great", true });
		Name1.Add(NameStruct{ "Greater", true });
		Name1.Add(NameStruct{ "Upper", true });
		Name1.Add(NameStruct{ "Little", true });
		Name1.Add(NameStruct{ "Cold", true });
		Name1.Add(NameStruct{ "Old", true });
		Name1.Add(NameStruct{ "New", true });
		Name1.Add(NameStruct{ "Lower", true });
		Name1.Add(NameStruct{ "Honking", true });
		Name1.Add(NameStruct{ "Huge", true });
		Name1.Add(NameStruct{ "Vast", true });
		Name1.Add(NameStruct{ "Sodden", true });
		Name1.Add(NameStruct{ "Moist", true });
		Name1.Add(NameStruct{ "North", true });
		Name1.Add(NameStruct{ "South", true });
		Name1.Add(NameStruct{ "East", true });
		Name1.Add(NameStruct{ "West", true });
		Name1.Add(NameStruct{ "Northern", true });
		Name1.Add(NameStruct{ "Eastern", true });
		Name1.Add(NameStruct{ "Southern", true });
		Name1.Add(NameStruct{ "Western", true });
		Name1.Add(NameStruct{ "Twin", true });
		Name1.Add(NameStruct{ "Outer", true });
		Name1.Add(NameStruct{ "Inner", true });

		TArray<NameStruct> Name2;
		Name2.Add(NameStruct{ "Butts", true });
		Name2.Add(NameStruct{ "Arse", true });
		Name2.Add(NameStruct{ "Shitterton", false });
		Name2.Add(NameStruct{ "Nipsworth", false });
		Name2.Add(NameStruct{ "Tittersville", false });
		Name2.Add(NameStruct{ "Crapton", false });

		TArray<NameStruct> Name3;
		Name3.Add(NameStruct{ "End", false });
		Name3.Add(NameStruct{ "Cove", false });
		Name3.Add(NameStruct{ "Hump", false });
		Name3.Add(NameStruct{ "Upon", true });
		Name3.Add(NameStruct{ "Next", true });
		Name3.Add(NameStruct{ "On", true });

		TArray<NameStruct> Name4;
		Name4.Add(NameStruct{ "Sea", false });
		Name4.Add(NameStruct{ "Tweed", false });
		Name4.Add(NameStruct{ "Fart", false });
		Name4.Add(NameStruct{ "Fire", false });

		int32 i1 = FMath::RandRange(0, Name1.Num() - 1);
		int32 i2 = FMath::RandRange(0, Name2.Num() - 1);
		int32 i3 = FMath::RandRange(0, Name3.Num() - 1);
		int32 i4 = FMath::RandRange(0, Name4.Num() - 1);

		FString NewName = Name1[i1].Name;
		if (Name1[i1].CanLink)
		{
			NewName += " " + Name2[i2].Name;

			if (Name2[i2].CanLink)
			{

				NewName += " " + Name3[i3].Name;

				if (Name3[i3].CanLink)
				{

					NewName += " " + Name4[i4].Name;
				}
			}
		}

		self->mMapText = FText::FromString(NewName);
		self->UpdateRepresentation();
		});
#endif
}



IMPLEMENT_GAME_MODULE(FStationNamesModule, StationNames)