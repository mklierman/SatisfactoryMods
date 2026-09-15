#include "AutoSigns.h"
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
#include "Buildables/FGBuildableDroneStation.h"
#include <SessionSettings/SessionSettingsManager.h>
#include <Logging/StructuredLog.h>
#include "LocalUserInfo.h"
#include <Buildables/FGBuildableRailroadSignal.h>
#include <Patching/NativeHookManager.h>

#define LOCTEXT_NAMESPACE "FAutoSignsModule"
DEFINE_LOG_CATEGORY(AutoSigns_Log);

#pragma optimize("", off)
void FAutoSignsModule::StartupModule()
{
	AFGBuildableHologram* bh = GetMutableDefault<AFGBuildableHologram>();
	AFGBuildableWidgetSign* bws = GetMutableDefault<AFGBuildableWidgetSign>();
#if !WITH_EDITOR
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableWidgetSign::InitializeSignPrefabData, bws, [this](AFGBuildableWidgetSign* sign)
		{
			if (IsValid(sign))
			{
				InitializeSignPrefabData(sign);
			}
		});

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableHologram::ConfigureActor, bh, [this](const AFGBuildableHologram* self, class AFGBuildable* inBuildable)
		{
			if (IsValid(self) && IsValid(inBuildable))
			{
				ConfigureActor(self, inBuildable);
			}
		});

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableWidgetSign::UpdateSignElements, bws, [this](AFGBuildableWidgetSign* sign, FPrefabSignData& prefabSignData)
		{
			if (IsValid(sign))
			{
				UpdateSignElements(sign, prefabSignData);
			}
		});

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableWidgetSign::OnBuildEffectFinished, bws, [this](AFGBuildableWidgetSign* sign)
		{
			if (IsValid(sign))
			{
				//FPrefabSignData signData;
				//sign->GetSignPrefabData(signData);
				SetDefaultSignData(sign);
			}
		});
#endif
}

void FAutoSignsModule::ConfigureActor(const AFGBuildableHologram* self, class AFGBuildable* inBuildable)
{
	if (auto sign = Cast<AFGBuildableWidgetSign>(inBuildable))
	{
		auto holo = const_cast<AFGBuildableHologram*>(self);
		auto snappedTo = holo->GetSnappedBuilding();
		if (IsValid(snappedTo))
		{
			SignSnaps.Add(sign, snappedTo);
		}
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
				if (IsValid(recipe))
				{
					auto products = UFGRecipe::GetProducts(recipe);
					if (products.Num() > 0)
					{
						auto itemDesc = products[0].ItemClass;
						nameText = UFGItemDescriptor::GetItemName(itemDesc).ToString();
						iconId = GetIconIdForDescriptor(sign->GetWorld(), itemDesc, iconId);
					}
				}
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
			AFGBuildableDroneStation* droneStation = Cast<AFGBuildableDroneStation>(snapped);
			if (droneStation)
			{
				auto inventory = droneStation->GetInputInventory();
				if (!inventory || inventory->GetFirstIndexWithItem() == INDEX_NONE)
				{
					inventory = droneStation->GetOutputInventory();
				}

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
			if (snapped->GetClass()->GetName().Contains(TEXT("LoadBalancer")))
			{
				TSubclassOf<UFGItemDescriptor> filterItemDesc = nullptr;
				if (FProperty* filterProp = snapped->GetClass()->FindPropertyByName(TEXT("mFilteredItems")))
				{
					if (FArrayProperty* arrayProp = CastField<FArrayProperty>(filterProp))
					{
						FScriptArrayHelper arrayHelper(arrayProp, arrayProp->ContainerPtrToValuePtr<void>(snapped));
						for (int32 i = 0; i < arrayHelper.Num(); ++i)
						{
							if (FClassProperty* classProp = CastField<FClassProperty>(arrayProp->Inner))
							{
								if (UClass* itemClass = Cast<UClass>(classProp->GetPropertyValue(arrayHelper.GetRawPtr(i))))
								{
									if (itemClass->GetName() != TEXT("FGNoneDescriptor"))
									{
										filterItemDesc = itemClass;
										break;
									}
								}
							}
						}
					}
				}

				if (!filterItemDesc)
				{
					if (FProperty* singleProp = snapped->GetClass()->FindPropertyByName(TEXT("mFilteredItem")))
					{
						if (FClassProperty* classProp = CastField<FClassProperty>(singleProp))
						{
							if (UClass* itemClass = Cast<UClass>(classProp->GetPropertyValue_InContainer(snapped)))
							{
								if (itemClass->GetName() != TEXT("FGNoneDescriptor"))
								{
									filterItemDesc = itemClass;
								}
							}
						}
					}
				}

				if (filterItemDesc)
				{
					nameText = UFGItemDescriptor::GetItemName(filterItemDesc).ToString();
					iconId = GetIconIdForDescriptor(sign->GetWorld(), filterItemDesc, iconId);
				}
				else
				{
					TArray<AFGBuildable*> modulesToCheck;
					if (UFunction* func = snapped->FindFunction(TEXT("GetGroupModules")))
					{
						struct { TArray<AFGBuildable*> ReturnValue; } params;
						snapped->ProcessEvent(func, &params);
						for (auto mod : params.ReturnValue)
						{
							if (IsValid(mod))
							{
								modulesToCheck.AddUnique(mod);
							}
						}
					}

					if (FProperty* leaderProp = snapped->GetClass()->FindPropertyByName(TEXT("GroupLeader")))
					{
						if (FObjectProperty* objProp = CastField<FObjectProperty>(leaderProp))
						{
							if (auto leaderBuildable = Cast<AFGBuildable>(objProp->GetObjectPropertyValue_InContainer(snapped)))
							{
								modulesToCheck.AddUnique(leaderBuildable);
							}
						}
					}
					modulesToCheck.AddUnique(snapped);

					for (auto mod : modulesToCheck)
					{
						UFGInventoryComponent* inventory = nullptr;
						if (FProperty* invProp = mod->GetClass()->FindPropertyByName(TEXT("mBufferInventory")))
						{
							if (FObjectProperty* objProp = CastField<FObjectProperty>(invProp))
							{
								inventory = Cast<UFGInventoryComponent>(objProp->GetObjectPropertyValue_InContainer(mod));
							}
						}

						if (!inventory)
						{
							inventory = mod->FindComponentByClass<UFGInventoryComponent>();
						}

						if (inventory)
						{
							auto firstIdx = inventory->GetFirstIndexWithItem();
							FInventoryStack stack;
							if (inventory->GetStackFromIndex(firstIdx, stack))
							{
								auto itemDesc = stack.Item.GetItemClass();
								if (itemDesc)
								{
									nameText = UFGItemDescriptor::GetItemName(itemDesc).ToString();
									iconId = GetIconIdForDescriptor(sign->GetWorld(), itemDesc, iconId);
									break;
								}
							}
						}
					}
				}
			}

			signData.TextElementData["Name"] = nameText;
			signData.IconElementData["Icon"] = iconId;
			if (DefaultTextElementToDataMap.Num() > 0 || DefaultIconElementToDataMap.Num() > 0)
			{
				USessionSettingsManager* SessionSettings = sign->GetWorld()->GetSubsystem<USessionSettingsManager>();
				auto copyLayout = SessionSettings->GetBoolOptionValue("AutoSigns.CopyLayout");
				if (copyLayout)
				{
					signData.PrefabLayout = LastSignData.PrefabLayout;
					signData.ForegroundColor = LastSignData.ForegroundColor;
					signData.BackgroundColor = LastSignData.BackgroundColor;
					signData.AuxiliaryColor = LastSignData.AuxiliaryColor;
					signData.Emissive = LastSignData.Emissive;
					signData.Glossiness = LastSignData.Glossiness;
				}
			}
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

void FAutoSignsModule::UpdateSignElements(AFGBuildableWidgetSign* sign, FPrefabSignData& prefabSignData)
{
	//UE_LOGFMT(AutoSigns_Log, Display, "UpdateSignElements");

	if (DefaultTextElementToDataMap.Num() > 1 || DefaultIconElementToDataMap.Num() > 0)
	{
		bool textIsEqual = prefabSignData.TextElementData.OrderIndependentCompareEqual(DefaultTextElementToDataMap);
		bool iconIsEqual = prefabSignData.IconElementData.OrderIndependentCompareEqual(DefaultIconElementToDataMap);
		if (textIsEqual && iconIsEqual)
			return;

		LastSignData = prefabSignData;
	}
}

void FAutoSignsModule::SetDefaultSignData(AFGBuildableWidgetSign* sign)
{
	//UE_LOGFMT(AutoSigns_Log, Display, "SetDefaultSignData");
	if (Cast<AFGBuildableRailroadSignal>(sign))
	{
		return;
	}
	if (DefaultTextElementToDataMap.Num() < 1 && DefaultIconElementToDataMap.Num() < 1)
	{
		sign->GetDefaultSignMaps(DefaultTextElementToDataMap, DefaultIconElementToDataMap);
	}
}

void FAutoSignsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

#pragma optimize("", on)
IMPLEMENT_MODULE(FAutoSignsModule, AutoSigns)