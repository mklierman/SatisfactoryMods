// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoSigns.h"
#include "Patching/NativeHookManager.h"
#include "Hologram/FGBuildableHologram.h"
#include "Buildables/FGBuildableWidgetSign.h"
#include "FGSignTypes.h"
#include "FGIconLibrary.h"
#include "Buildables/FGBuildableFactory.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "Buildables/FGBuildableSignBase.h"
#include "Buildables/FGBuildableResourceExtractor.h"
#include "Resources/FGResourceNode.h"
#include "Buildables/FGBuildableStorage.h"
#include "Buildables/FGBuildableTrainPlatformCargo.h"
#include "Buildables/FGBuildableDockingStation.h"
#include <Logging/StructuredLog.h>

#define LOCTEXT_NAMESPACE "FAutoSignsModule"
DEFINE_LOG_CATEGORY(AutoSigns_Log);

#pragma optimize("", off)
void FAutoSignsModule::StartupModule()
{
	AFGBuildableHologram* bh = GetMutableDefault<AFGBuildableHologram>();
	AFGBuildableWidgetSign* bws = GetMutableDefault<AFGBuildableWidgetSign>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableWidgetSign::InitializeSignPrefabData, bh, [this](AFGBuildableWidgetSign* sign)
		{
			InitializeSignPrefabData(sign);
		});

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableHologram::ConfigureActor, bh, [this](const AFGBuildableHologram* self, class AFGBuildable* inBuildable)
		{
			ConfigureActor(self, inBuildable);
		});


}

void FAutoSignsModule::ConfigureActor(const AFGBuildableHologram* self, class AFGBuildable* inBuildable)
{
	if (auto sign = Cast<AFGBuildableWidgetSign>(inBuildable))
	{
		auto holo = const_cast<AFGBuildableHologram*>(self);
		auto snappedTo = holo->GetSnappedBuilding();
		SignSnaps.Add(sign, snappedTo);
		return;
	}
}

void FAutoSignsModule::InitializeSignPrefabData(AFGBuildableWidgetSign* sign)
{
	if (SignSnaps.Contains(sign))
	{
		AFGBuildable* snapped = *SignSnaps.Find(sign);
		if (snapped)
		{
			FPrefabSignData signData;
			sign->GetSignPrefabData(signData);
			FString nameText = *signData.TextElementData.Find("Name");
			int32 iconId = *signData.IconElementData.Find("Icon");
			AFGBuildableManufacturer* manufacturer = Cast<AFGBuildableManufacturer>(snapped);
			if (manufacturer)
			{
				auto recipe = manufacturer->GetCurrentRecipe();
				auto products = UFGRecipe::GetProducts(recipe);
				auto itemDesc = products[0].ItemClass;
				nameText = UFGItemDescriptor::GetItemName(itemDesc).ToString();
				iconId = GetIconIdForDescriptor(sign->GetWorld(), itemDesc, iconId);
			}
			AFGBuildableResourceExtractor* extractor = Cast<AFGBuildableResourceExtractor>(snapped);
			if (extractor)
			{
				auto extractableRes = extractor->mExtractableResource;
				auto node = Cast<AFGResourceNode>(extractableRes);
				if (node)
				{
					auto resourceDesc = node->GetResourceClass();

					nameText = UFGItemDescriptor::GetItemName(resourceDesc).ToString();
					iconId = GetIconIdForDescriptor(sign->GetWorld(), resourceDesc, iconId);
				}
			}
			AFGBuildableStorage* storage = Cast<AFGBuildableStorage>(snapped);
			if (storage)
			{
				auto inventory = storage->GetStorageInventory();
				if (inventory)
				{
					auto firstIdx = inventory->GetFirstIndexWithItem();
					FInventoryStack stack;
					if (inventory->GetStackFromIndex(firstIdx, stack))
					{
						auto itemDesc = stack.Item.GetItemClass();
						nameText = UFGItemDescriptor::GetItemName(itemDesc).ToString();
						iconId = GetIconIdForDescriptor(sign->GetWorld(), itemDesc, iconId);
					}
				}
			}
			AFGBuildableTrainPlatformCargo* cargoStation = Cast< AFGBuildableTrainPlatformCargo>(snapped);
			if (cargoStation)
			{
				auto inventory = cargoStation->GetInventory();
				if (inventory)
				{
					auto firstIdx = inventory->GetFirstIndexWithItem();
					FInventoryStack stack;
					if (inventory->GetStackFromIndex(firstIdx, stack))
					{
						auto itemDesc = stack.Item.GetItemClass();
						nameText = UFGItemDescriptor::GetItemName(itemDesc).ToString();
						iconId = GetIconIdForDescriptor(sign->GetWorld(), itemDesc, iconId);
					}
				}
			}
			AFGBuildableDockingStation* dockingStation = Cast< AFGBuildableDockingStation>(snapped);
			if (dockingStation)
			{
				auto inventory = dockingStation->GetInventory();
				if (inventory)
				{
					auto firstIdx = inventory->GetFirstIndexWithItem();
					FInventoryStack stack;
					if (inventory->GetStackFromIndex(firstIdx, stack))
					{
						auto itemDesc = stack.Item.GetItemClass();
						nameText = UFGItemDescriptor::GetItemName(itemDesc).ToString();
						iconId = GetIconIdForDescriptor(sign->GetWorld(), itemDesc, iconId);
					}
				}
			}

			signData.TextElementData["Name"] = nameText;
			signData.IconElementData["Icon"] = iconId;

			sign->SetPrefabSignData(signData);
			SignSnaps.Remove(sign);
		}
	}
}

int32 FAutoSignsModule::GetIconIdForDescriptor(UObject* worldContext, TSubclassOf<UFGItemDescriptor> descriptor, int32 fallback = 0)
{
	TArray< FIconData > iconData;

	UFGIconLibrary::GetAllIconDataForType(worldContext, EIconType::ESIT_Part, true, iconData);
	for (auto data : iconData)
	{
		if (data.ItemDescriptor.Get() == descriptor)
		{
			return data.ID;
		}
	}

	UFGIconLibrary::GetAllIconDataForType(worldContext, EIconType::ESIT_Equipment, true, iconData);
	for (auto data : iconData)
	{
		if (data.ItemDescriptor.Get() == descriptor)
		{
			return data.ID;
		}
	}

	return fallback;
}

void FAutoSignsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

#pragma optimize("", on)
IMPLEMENT_MODULE(FAutoSignsModule, AutoSigns)