


#include "StationNames_BPFL.h"
#include "FGMapAreaTexture.h"
#include "FGMapArea.h"
#include "FGInventoryComponent.h"
#include "FGPlayerController.h"
#include "Resources\FGItemDescriptor.h"
#include "FGBlueprintFunctionLibrary.h"

void UStationNames_BPFL::AutoSetDockingStationName(AFGBuildableDockingStation* DockingStation, FString& OutMapArea, FString& OutLoadMode, FString& OutItemName)
{
	FText areaText = FText::FromString("Empty");
	AFGPlayerController* player = UFGBlueprintFunctionLibrary::GetLocalPlayerController(DockingStation->GetWorld());
	if (player)
	{
		UFGMapAreaTexture* mat = player->GetMapAreaTexture(); if (mat)
		{
			//UFGMapAreaTexture* mat = UFGMapAreaTexture::StaticClass();
			auto mapArea = mat->GetMapAreaForWorldLocation(DockingStation->GetActorLocation());
			if (mapArea)
			{
				areaText = UFGMapArea::GetAreaDisplayName(mapArea);
			}
		}
	}

	FString loadModeString = DockingStation->GetIsInLoadMode() ? "Supplying" : "Demanding"; //Supply & Demand
	//FText loadModeText = FText::FromString(loadModeString);
	auto invComp = DockingStation->GetInventory();
	auto firstItemIndex = invComp->GetFullestStackIndex();
	FString inventoryText = "Nothing";
	if (firstItemIndex >= 0)
	{
		//FInventoryStack invStack = invComp->GetStackFromIndex(firstItemIndex);
		//inventoryText = UFGItemDescriptor::GetItemName(invStack.Item.ItemClass.Get()).ToString();
		//inventoryText = invStack.Item.ItemClass.Get()->GetFName().ToString();
		//inventoryText = FText::FromString(inventoryString);
	}
	OutMapArea = areaText.ToString();
	OutLoadMode = loadModeString;
	OutItemName = inventoryText;
	//UE_LOG(StationNames_Log, Display, TEXT("Map Area: %s | Load Mode: %s | Item: %s"), *areaText.ToString(), *loadModeText.ToString(), *inventoryText.ToString());
    //return "Map Area: " + areaText.ToString() + " | Load Mode : " + loadModeText.ToString() + " | Item : " + inventoryText;
}

FString UStationNames_BPFL::GetRandomizedName()
{
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
    return NewName;
}

TSubclassOf<UFGMapArea> UStationNames_BPFL::GetMapAreaForWorldLocation(UFGMapAreaTexture* mapAreaTexture, FVector worldLocation)
{
	return mapAreaTexture->GetMapAreaForWorldLocation(worldLocation);
}
