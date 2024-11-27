// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerStackSizer.h"
#include <FGCharacterPlayer.h>
#include <NativeHookManager.h>
#include <SessionSettings/SessionSettingsManager.h>
#include "FGInventoryLibrary.h"
#include <FGInventoryComponentTrash.h>

#define LOCTEXT_NAMESPACE "FPlayerStackSizerModule"

#pragma optimize("", off)
void FPlayerStackSizerModule::StartupModule()
{
#if !WITH_EDITOR
	// Get player inventory and set all slot sizes to 500
	AFGCharacterPlayer* cdo = GetMutableDefault<AFGCharacterPlayer>();

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGCharacterPlayer::BeginPlay, cdo, [=](AFGCharacterPlayer* self)
		{
			USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			auto stackSizeFloat = SessionSettings->GetFloatOptionValue("PlayerStackSizer.StackSize");
			int32 stackSizeInt = FMath::FloorToInt(stackSizeFloat);
			auto inventory = self->GetInventory();
			auto size = inventory->GetSizeLinear();
			for (int i = 0; i < size; i++)
			{
				inventory->AddArbitrarySlotSize(i, stackSizeInt);
			}
		});


	SUBSCRIBE_METHOD_AFTER(AFGCharacterPlayer::OnInventorySlotsUnlocked, [=](AFGCharacterPlayer* self, const int32 newUnlockedSlots)
		{
			USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			auto stackSizeFloat = SessionSettings->GetFloatOptionValue("PlayerStackSizer.StackSize");
			int32 stackSizeInt = FMath::FloorToInt(stackSizeFloat);
			auto inventory = self->GetInventory();
			auto size = inventory->GetSizeLinear();
			for (int i = 0; i < size; i++)
			{
				inventory->AddArbitrarySlotSize(i, stackSizeInt);
			}
		});

	SUBSCRIBE_METHOD(UFGInventoryLibrary::MoveInventoryItem, [this](auto& scope, UFGInventoryComponent* sourceComponent, const int32 sourceIdx, UFGInventoryComponent* destinationComponent, const int32 destinationIdx)
		{
            int32 result = this->MoveInventoryItems(sourceComponent, sourceIdx, destinationComponent, destinationIdx);
            if (result == 0)
            {
                return;
            }
            else if (result == 1)
            {
                scope.Override(true);
            }
            else
            {
                scope.Override(false);
            }



		});
#endif
}

void FPlayerStackSizerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

int32 FPlayerStackSizerModule::MoveInventoryItems(UFGInventoryComponent* sourceComponent, const int32 sourceIdx, UFGInventoryComponent* destinationComponent, const int32 destinationIdx)
{
    // return 0; return;
    // return 1; scope.Override(true);
    // return 2; scope.Override(false);

    // General checks. If any fail we can just default to vanilla
    if (!sourceComponent || !destinationComponent
        || sourceIdx < 0 || destinationIdx < 0
        || sourceIdx >= sourceComponent->mInventoryStacks.Num()
        || destinationIdx >= destinationComponent->mInventoryStacks.Num()
        || sourceComponent == destinationComponent
        || !sourceComponent->HasAuthority()
        || !destinationComponent->HasAuthority()
        || !sourceComponent->mCanBeRearrange)
    {
        return 0;
    }

    auto trashSlot = Cast<UFGInventoryComponentTrash>(destinationComponent);
    if (trashSlot)
    {
        return 0;
    }

    FInventoryStack sourceStack;
    sourceStack = sourceComponent->GetStackFromIndex(sourceIdx);
    FInventoryStack destinationStack;
    destinationStack = destinationComponent->GetStackFromIndex(destinationIdx);
    //auto sourceItemStackSize = sourceComponent->GetSlotSizeForItem(sourceIdx);
    auto sourceItemStackSize = sourceStack.NumItems;
    auto itemClass = sourceStack.Item.ItemClass;
    auto itemStackSize = UFGItemDescriptor::GetStackSize(itemClass);
    auto sourceNumItems = sourceStack.NumItems;
    auto destNumItems = destinationStack.NumItems;
    if (sourceNumItems <= 0)
    {
        return 0;
    }


    // Same slot size, let vanilla handle it
    // If a slot doesn't have an arbitrary size this will give 0. Need to get the stack size for the item type
    auto sourceSlotSize = sourceComponent->mArbitrarySlotSizes[sourceIdx];
    auto destSlotSize = destinationComponent->mArbitrarySlotSizes[destinationIdx];
    auto sSlotSize = sourceComponent->GetSlotSize(sourceIdx);
    auto dSlotSize = destinationComponent->GetSlotSize(destinationIdx);
    if (destSlotSize == 0)
    {
        destSlotSize = itemStackSize;
    }

    if (sourceNumItems <= destSlotSize || destSlotSize >= sourceSlotSize)
    {
        return 0;
    }
    if (destSlotSize < sourceSlotSize)
    {
        if (destinationStack.Item.ItemClass != nullptr && itemClass != destinationStack.Item.ItemClass)
        {
            return 2;
        }

        auto destNumItems = destinationStack.NumItems;
        if (sourceNumItems + destNumItems > destSlotSize)
        {
            // This works to prevent the move, but we want to split the stack if we can
            //return 2;

            sourceComponent->RemoveFromIndex(sourceIdx, destSlotSize - destNumItems);
            FInventoryStack newStack;
            newStack.Item = sourceStack.Item;
            newStack.NumItems = destSlotSize;
            destinationStack.Item = sourceStack.Item;
            destinationStack.NumItems = destSlotSize;
            destinationComponent->RemoveFromIndex(destinationIdx, destNumItems);
            destinationComponent->AddStackToIndex(destinationIdx, newStack);
            return 1;
        }
    }
    return 0;
}
#pragma optimize("", on)

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPlayerStackSizerModule, PlayerStackSizer)