#include "LBBuild_ModularLoadBalancer.h"

#include "FGColoredInstanceMeshProxy.h"
#include "FGFactoryConnectionComponent.h"
#include "FGInventoryLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "FGOutlineComponent.h"
#include "LBDefaultRCO.h"
#include "LoadBalancersModule.h"
#include <Net/UnrealNetwork.h>
#include <Kismet/GameplayStatics.h>
#include <Buildables/FGBuildableConveyorBelt.h>

DEFINE_LOG_CATEGORY(LoadBalancers_Log);
//DEFINE_LOG_CATEGORY(LogGame);
//DEFINE_LOG_CATEGORY(LogBuilding);

#pragma optimize("", off)

void FLBBalancerData::GetInputBalancers(TArray<ALBBuild_ModularLoadBalancer*>& Out)
{
    if (mConnectedInputs.Num() > 0)
    {
        for (TWeakObjectPtr<ALBBuild_ModularLoadBalancer> Balancer : mConnectedInputs)
        {
            if (Balancer != nullptr && Balancer.IsValid())
            {
                Out.AddUnique(Balancer.Get());
            }
        }
    }
}

bool FLBBalancerData::HasAnyValidFilter() const
{
    if(mFilterMap.Num() >= 0)
    {
        TArray<TSubclassOf<UFGItemDescriptor>> Keys;
        mFilterMap.GenerateKeyArray(Keys);
        return !(mFilterMap.Num() == 1 && Keys.Contains(UFGNoneDescriptor::StaticClass()));
    }
    return false;
}

bool FLBBalancerData::HasAnyValidOverflow() const
{
    if(mOverflowBalancer.Num() >= 0)
    {
        for (TWeakObjectPtr<ALBBuild_ModularLoadBalancer> Overflower : mOverflowBalancer)
        {
            if(Overflower.IsValid())
            {
                return true;
            }
        }
    }
    return false;
}

void FLBBalancerData::SetFilterItemForBalancer(ALBBuild_ModularLoadBalancer* Balancer, TSubclassOf<UFGItemDescriptor> Item)
{
    if(HasItemFilterBalancer(Item))
    {
        mFilterMap[Item].mBalancer.AddUnique(Balancer);
    }
    else
    {
        mFilterMap.Add(Item, FLBBalancerData_Filters(Balancer));
    }
    Balancer->mBufferInventory->SetLocked(false);
}

void FLBBalancerData::RemoveBalancer(ALBBuild_ModularLoadBalancer* Balancer, TSubclassOf<UFGItemDescriptor> OldItem)
{
    if(HasItemFilterBalancer(OldItem))
    {
        if(mFilterMap[OldItem].mBalancer.Contains(Balancer))
        {
            mFilterMap[OldItem].mBalancer.Remove(Balancer);
        }
        if(mFilterMap[OldItem].mBalancer.Num() == 0)
        {
            mFilterMap.Remove(OldItem);
        }
    }

    if(Balancer)
    {
        if(mConnectedOutputs.Contains(Balancer))
        {
            mConnectedOutputs.Remove(Balancer);
        }

        if(mConnectedInputs.Contains(Balancer))
        {
            mConnectedInputs.Remove(Balancer);
        }

        if(Balancer->IsOverflowModule())
        {
            if(mOverflowBalancer.Contains(Balancer))
            {
                mOverflowBalancer.Remove(Balancer);
            }
        }
    }
}

bool FLBBalancerData::HasItemFilterBalancer(TSubclassOf<UFGItemDescriptor> Item) const
{
    if (Item)
    {
        return mFilterMap.Contains(Item);
    }
    return false;
}

ALBBuild_ModularLoadBalancer::ALBBuild_ModularLoadBalancer()
{
    if (!mBufferInventory)
    {
        mBufferInventory = CreateDefaultSubobject<UFGInventoryComponent>(FName("mBufferInventory"), false);
        mBufferInventory->Resize(4);
    }
}

void ALBBuild_ModularLoadBalancer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALBBuild_ModularLoadBalancer, mGroupModules);
    DOREPLIFETIME(ALBBuild_ModularLoadBalancer, GroupLeader);
    DOREPLIFETIME(ALBBuild_ModularLoadBalancer, mFilteredItems);
}

void ALBBuild_ModularLoadBalancer::BeginPlay()
{
    Super::BeginPlay();
	InitializeModule();

    //Fix for Z-Fighting
    auto meshes = GetComponents();
    for (auto mesh : meshes)
    {
        auto sMesh = Cast<UStaticMeshComponent>(mesh);
        if (sMesh)
        {
            float randomFloat = UKismetMathLibrary::RandomFloatInRange(-0.001, 0.001);
            sMesh->AddRelativeLocation(FVector(randomFloat, randomFloat, randomFloat));
            sMesh->AddRelativeRotation(FRotator(randomFloat, randomFloat, randomFloat));
        }
    }
}

void ALBBuild_ModularLoadBalancer::InitializeModule()
{
    if (HasAuthority())
    {
        if (!GroupLeader)
        {
            GroupLeader = this;
        }

        if (mFilteredItem && mFilteredItem != UFGNoneDescriptor::StaticClass())
        {
            mFilteredItems.SetNum(1);
            mFilteredItems[0] = mFilteredItem;
            mFilteredItem = nullptr;
        }

        SetupInventory();

        if (MyOutputConnection && GetBufferInventory())
        {
            mBufferInventory->SetLocked(false);
            int32 Index = MyOutputConnection->GetInventoryAccessIndex();
            if (GetBufferInventory()->IsIndexEmpty(Index))
            {
                MyOutputConnection->SetInventoryAccessIndex(-1);
            }
        }

        ApplyGroupModule();
        for (TPair<TSubclassOf<UFGItemDescriptor>,FLBBalancerData_Filters> item : GroupLeader->mNormalLoaderData.mFilterMap)
        {
            if (item.Key != UFGNoneDescriptor::StaticClass() && !GroupLeader->mNormalLoaderData.mFilterInputMap.Contains(item.Key))
            {
                GroupLeader->mNormalLoaderData.mFilterInputMap.Add(item.Key);
            }
        }
    }
}


EProductionStatus ALBBuild_ModularLoadBalancer::GetProductionIndicatorStatus() const
{
    return EProductionStatus::IS_PRODUCING;
}

void ALBBuild_ModularLoadBalancer::SetupInventory()
{
    if (GetBufferInventory())
    {
        if(!IsFilterModule())
        {
            if(MyOutputConnection)
            {
                MyOutputConnection->SetInventory(GetBufferInventory());
            }

            GetBufferInventory()->OnItemRemovedDelegate.AddUniqueDynamic(this, &ALBBuild_ModularLoadBalancer::OnOutputItemRemoved);
            GetBufferInventory()->Resize(mSlotsInBuffer);

            for (int i = 0; i < GetBufferInventory()->GetSizeLinear(); ++i)
            {
                if (GetBufferInventory()->IsValidIndex(i))
                {
                    GetBufferInventory()->AddArbitrarySlotSize(i, mOverwriteSlotSize);
                }
            }
        }
        else
        {
            if(MyOutputConnection)
            {
                MyOutputConnection->SetInventory(GetBufferInventory());
            }

            GetBufferInventory()->OnItemRemovedDelegate.AddUniqueDynamic(this, &ALBBuild_ModularLoadBalancer::OnOutputItemRemoved);
            GetBufferInventory()->Resize(mFilteredItems.Num());

            for (int i = 0; i < GetBufferInventory()->GetSizeLinear(); ++i)
            {
                if (GetBufferInventory()->IsValidIndex(i))
                {
                    GetBufferInventory()->AddArbitrarySlotSize(i, mOverwriteSlotSize);
                    GetBufferInventory()->SetAllowedItemOnIndex(i, mFilteredItems[i]);
                }
            }
        }
        GetBufferInventory()->SetLocked(false);
    }
}

UFGInventoryComponent* ALBBuild_ModularLoadBalancer::GetBufferInventory()
{
    return mBufferInventory;

    TSet<UActorComponent*> components = GetComponents();
    for (UActorComponent* ComponentsByClass : components)
    {
        if (UFGInventoryComponent* InventoryComponent = Cast<UFGInventoryComponent>(ComponentsByClass))
        {
            return InventoryComponent;
        }
    }
    return nullptr;
}

void ALBBuild_ModularLoadBalancer::SetFilteredItem(TSubclassOf<UFGItemDescriptor> ItemClass)
{
    if(!ItemClass || mFilteredItems.Contains(ItemClass))
    {
        return;
    }

    if(mLoaderType == ELoaderType::Programmable)
    {
        if(mFilteredItems.Num() >= mMaxFilterableItems)
        {
            return;
        }
    }

    if (HasAuthority())
    {
        if (IsFilterModule() && GetBufferInventory())
        {
            GetBufferInventory()->SetLocked(false);
            if (GroupLeader)
            {
                if (mLoaderType == ELoaderType::Filter)
                {
                    TSubclassOf<UFGItemDescriptor> OldItem = mFilteredItems[0];
                    mFilteredItems[0] = ItemClass;
                    GroupLeader->mNormalLoaderData.RemoveBalancer(this, OldItem);
                    GroupLeader->mNormalLoaderData.mFilterInputMap.Add(ItemClass, 0);
                }
                else
                {
                    if (mFilteredItems[0] == UFGNoneDescriptor::StaticClass())
                    {
                        mFilteredItems[0] = ItemClass;
                    }
                    else
                    {
                        mFilteredItems.Add(ItemClass);
                    }
                }

                GroupLeader->mNormalLoaderData.SetFilterItemForBalancer(this, ItemClass);
            }
            SetupInventory();
        }
    }
    else if (ULBDefaultRCO* RCO = ULBDefaultRCO::Get(GetWorld()))
    {
        RCO->Server_SetFilteredItem(this, ItemClass);
    }
}

void ALBBuild_ModularLoadBalancer::RemoveFilteredItem(TSubclassOf<UFGItemDescriptor> ItemClass)
{
    if(!ItemClass || !mFilteredItems.Contains(ItemClass))
    {
        return;
    }

    if (HasAuthority())
    {
        if (IsFilterModule() && GetBufferInventory())
        {
            GetBufferInventory()->SetLocked(false);
            mFilteredItems.Remove(ItemClass);
            if (GroupLeader)
            {
                GroupLeader->mNormalLoaderData.RemoveBalancer(this, ItemClass);
            }

            if (mFilteredItems.Num() == 0)
            {
                mFilteredItems.SetNum(1);
                mFilteredItems[0] = UFGNoneDescriptor::StaticClass();
            }

            GroupLeader->mNormalLoaderData.RemoveBalancer(this, ItemClass);
            SetupInventory();
        }
    }
    else if (ULBDefaultRCO* RCO = ULBDefaultRCO::Get(GetWorld()))
    {
        RCO->Server_RemoveFilteredItem(this, ItemClass);
    }
}

void ALBBuild_ModularLoadBalancer::SetFilteredItems(TArray<TSubclassOf<UFGItemDescriptor>> Items)
{
    RemoveAllFilteredItems();
    for (TSubclassOf<UFGItemDescriptor> item : Items)
    {
        SetFilteredItem(item);
    }
}

void ALBBuild_ModularLoadBalancer::RemovedFilteredItems(TArray<TSubclassOf<UFGItemDescriptor>> Items)
{
    for (TSubclassOf<UFGItemDescriptor> item : Items)
    {
        RemoveFilteredItem(item);
    }
}

void ALBBuild_ModularLoadBalancer::RemoveAllFilteredItems()
{
    TArray<TSubclassOf<UFGItemDescriptor>> ItemsToRemove = mFilteredItems;
    for (TSubclassOf<UFGItemDescriptor> item : ItemsToRemove)
    {
        RemoveFilteredItem(item);
    }
}

void ALBBuild_ModularLoadBalancer::ApplyLeader()
{
    if (HasAuthority())
    {
        GroupLeader = this;

        for (ALBBuild_ModularLoadBalancer* ModularLoadBalancer : GetGroupModules())
        {
            if (ModularLoadBalancer)
            {
                ModularLoadBalancer->GroupLeader = this;
                ModularLoadBalancer->mNormalLoaderData = mNormalLoaderData;
                ModularLoadBalancer->ForceNetUpdate();
            }
        }

        UpdateCache();
        ForceNetUpdate();
    }
}

TArray<ALBBuild_ModularLoadBalancer*> ALBBuild_ModularLoadBalancer::GetGroupModules() const
{
    if (!GroupLeader)
    {
        return {};
    }

    TArray<ALBBuild_ModularLoadBalancer*> GroupModules;
    TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> Array = GroupLeader->mGroupModules;

    if (Array.Num() > 0)
    {
        for (TWeakObjectPtr<ALBBuild_ModularLoadBalancer> ModularLoadBalancer : Array)
        {
            if (ModularLoadBalancer.IsValid())
            {
                GroupModules.Add(ModularLoadBalancer.Get());
            }
        }
    }
    return GroupModules;
}

bool ALBBuild_ModularLoadBalancer::SendToOverflowBalancer(FInventoryItem Item) const
{
    if (!GroupLeader)
    {
        return false;
    }

    // Early return if no overflow module
    if (!HasOverflowModule())
    {
        return false;
    }

    // Get or create indexing for this item type
    FLBBalancerIndexing* Indexing = GroupLeader->mNormalLoaderData.mIndexMapping.Find(Item.GetItemClass());
    if (!Indexing)
    {
        GroupLeader->mNormalLoaderData.mIndexMapping.Add(Item.GetItemClass(), FLBBalancerIndexing());
        Indexing = GroupLeader->mNormalLoaderData.mIndexMapping.Find(Item.GetItemClass());
        if (!Indexing)
        {
            return false;
        }
    }

    // Initialize overflow index if needed
    if (Indexing->mOverflowIndex == -1)
    {
        Indexing->mOverflowIndex = 0;
    }

    // Get the overflow balancers array
    const TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>>& OverflowBalancers = 
        GroupLeader->mNormalLoaderData.mOverflowBalancer;
    
    if (OverflowBalancers.Num() == 0)
    {
        return false;
    }

    // Try to find a valid overflow balancer with available space
    const int32 StartIndex = Indexing->mOverflowIndex;
    int32 CurrentIndex = StartIndex;
    
    do
    {
        if (!OverflowBalancers.IsValidIndex(CurrentIndex))
        {
            CurrentIndex = 0;
            continue;
        }

        TWeakObjectPtr<ALBBuild_ModularLoadBalancer> OverflowBalancer = OverflowBalancers[CurrentIndex];
        CurrentIndex++;
        
        // Wrap around to beginning if we've reached the end
        if (CurrentIndex >= OverflowBalancers.Num())
        {
            CurrentIndex = 0;
        }

        if (OverflowBalancer.IsValid() && OverflowBalancer->GetBufferInventory())
        {
            if (GetNumItems(OverflowBalancer->GetBufferInventory(), Item.GetItemClass()) < mOverwriteSlotSize)
            {
                // Update the index for next time
                Indexing->mOverflowIndex = CurrentIndex;
                
                // Add item to the overflow balancer
                OverflowBalancer->GetBufferInventory()->AddItem(Item);
                return true;
            }
        }
    } while (CurrentIndex != StartIndex); // Stop if we've checked all balancers

    return false;
}

bool ALBBuild_ModularLoadBalancer::SendToFilterBalancer(FInventoryItem Item) const
{
    if (!GroupLeader)
    {
        return false;
    }

    // Early return if no filter module or no filter balancer for this item
    if (!HasFilterModule() || !GroupLeader->mNormalLoaderData.HasItemFilterBalancer(Item.GetItemClass()))
    {
        return false;
    }

    // Get or create indexing for this item type
    FLBBalancerIndexing* Indexing = GroupLeader->mNormalLoaderData.mIndexMapping.Find(Item.GetItemClass());
    if (!Indexing)
    {
        GroupLeader->mNormalLoaderData.mIndexMapping.Add(Item.GetItemClass(), FLBBalancerIndexing());
        Indexing = GroupLeader->mNormalLoaderData.mIndexMapping.Find(Item.GetItemClass());
        if (!Indexing)
        {
            return false;
        }
    }

    // Initialize filter index if needed
    if (Indexing->mFilterIndex == -1)
    {
        Indexing->mFilterIndex = 0;
    }

    // Get the filter balancers for this item type
    const TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>>& FilterBalancers = 
        GroupLeader->mNormalLoaderData.mFilterMap[Item.GetItemClass()].mBalancer;
    
    if (FilterBalancers.Num() == 0)
    {
        return false;
    }

    // Try to find a valid filter balancer with available space
    const int32 StartIndex = Indexing->mFilterIndex;
    int32 CurrentIndex = StartIndex;
    
    do
    {
        if (!FilterBalancers.IsValidIndex(CurrentIndex))
        {
            CurrentIndex = 0;
            continue;
        }

        TWeakObjectPtr<ALBBuild_ModularLoadBalancer> FilterBalancer = FilterBalancers[CurrentIndex];
        CurrentIndex++;
        
        // Wrap around to beginning if we've reached the end
        if (CurrentIndex >= FilterBalancers.Num())
        {
            CurrentIndex = 0;
        }

        if (FilterBalancer.IsValid() && FilterBalancer->GetBufferInventory())
        {
            if (GetNumItems(FilterBalancer->GetBufferInventory(), Item.GetItemClass()) < mOverwriteSlotSize)
            {
                // Update the index for next time
                Indexing->mFilterIndex = CurrentIndex;
                
                // Add item to the filter balancer
                FilterBalancer->GetBufferInventory()->AddItem(Item);
                return true;
            }
        }
    } while (CurrentIndex != StartIndex); // Stop if we've checked all balancers

    return false;
}

bool ALBBuild_ModularLoadBalancer::SendToNormalBalancer(FInventoryItem Item) const
{
    if(!GroupLeader)
    {
        return false;
    }

    if(GroupLeader->mNormalLoaderData.mConnectedOutputs.Num() > 0)
    {
        FLBBalancerIndexing* Indexing = GroupLeader->mNormalLoaderData.mIndexMapping.Find(Item.GetItemClass());
        if (!Indexing)
        {
            GroupLeader->mNormalLoaderData.mIndexMapping.Add(Item.GetItemClass(), FLBBalancerIndexing());
            Indexing = GroupLeader->mNormalLoaderData.mIndexMapping.Find(Item.GetItemClass());
        }

        if (Indexing)
        {
            if(Indexing->mNormalIndex == -1)
            {
                Indexing->mNormalIndex = 0;
            }
        }
        else
        {
            return false;
        }

        for (int i = 0; i < GroupLeader->mNormalLoaderData.mConnectedOutputs.Num(); i++)
        {
            TWeakObjectPtr<ALBBuild_ModularLoadBalancer> OverflowBalancer = GroupLeader->mNormalLoaderData.mConnectedOutputs.IsValidIndex(Indexing->mNormalIndex) ? GroupLeader->mNormalLoaderData.mConnectedOutputs[Indexing->mNormalIndex] : nullptr;

            Indexing->mNormalIndex++;
            if (!GroupLeader->mNormalLoaderData.mConnectedOutputs.IsValidIndex(Indexing->mNormalIndex))
            {
                Indexing->mNormalIndex = 0;
            }

            if(OverflowBalancer.IsValid())
            {
                if(OverflowBalancer->GetBufferInventory())
                {
                    if(GetNumItems(OverflowBalancer->GetBufferInventory(), Item.GetItemClass()) < mOverwriteSlotSize)
                    {
                        OverflowBalancer->GetBufferInventory()->AddItem(Item);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void ALBBuild_ModularLoadBalancer::CheckWeakArray(TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> weakObjects)
{
    for (auto& weakObj : weakObjects)
    {
        if (weakObj.IsValid())
        {
            continue;
        }
        else
        {
            weakObjects.Remove(weakObj);
        }
    }
}

int32 ALBBuild_ModularLoadBalancer::GetNumItems(UFGInventoryComponent* bufferInventory, TSubclassOf< UFGItemDescriptor > itemClass) const
{
    TArray< FInventoryStack > out_stacks;
    bufferInventory->GetInventoryStacks(out_stacks);
    if (out_stacks.Num() > 0)
    {
        for (FInventoryStack stack : out_stacks)
        {
            if (stack.Item.GetItemClass() == itemClass && stack.HasItems())
            {
                return stack.NumItems;
            }
        }
    }
    return 0;
}

void ALBBuild_ModularLoadBalancer::Factory_Tick(float dt)
{
    Super::Factory_Tick(dt);

    if (HasAuthority())
    {
        if (GetBufferInventory()->GetSizeLinear() != mSlotsInBuffer)
        {
            SetupInventory();
        }

        if (MyOutputConnection || MyInputConnection)
        {
            mTimer += dt;
            if (mTimer >= 1.f)
            {
                mTimer -= 1.f;
                AsyncTask(ENamedThreads::GameThread, [this]() { UpdateCache(); });
            }
        }

        if (MyOutputConnection && GetBufferInventory())
        {
            if(!GetBufferInventory()->IsValidIndex(MyOutputConnection->GetInventoryAccessIndex()) && MyOutputConnection->GetInventoryAccessIndex() != -1)
            {
                MyOutputConnection->SetInventoryAccessIndex(-1);
            }
        }
    }
}

void ALBBuild_ModularLoadBalancer::UpdateCache()
{
    if (!GroupLeader) 
    {
        return;
    }

    if (MyInputConnection) 
    {
        if (MyInputConnection->IsConnected()) 
        {
            GroupLeader->mNormalLoaderData.mConnectedInputs.AddUnique(this);
        }
        else 
        {
            GroupLeader->mNormalLoaderData.mConnectedInputs.Remove(this);
        }
    }

    if (MyOutputConnection && IsOverflowModule()) 
    {
        if (MyOutputConnection->IsConnected()) 
        {
            GroupLeader->mNormalLoaderData.mOverflowBalancer.AddUnique(this);
        } 
        else 
        {
            GroupLeader->mNormalLoaderData.mOverflowBalancer.Remove(this);
        }
    } 
    else if (MyOutputConnection && IsNormalModule()) 
    {
        if (MyOutputConnection->IsConnected()) 
        {
            GroupLeader->mNormalLoaderData.mConnectedOutputs.AddUnique(this);
        } 
        else 
        {
            GroupLeader->mNormalLoaderData.mConnectedOutputs.Remove(this);
        }
    }
}

void ALBBuild_ModularLoadBalancer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Apply new Leader to the group if the group has more as 1 module (single loader)
    // EndPlay is here a better use. We should do this better only if it's Destroyed on EndPlay (is also in no case to late to can be invalid)
    if (EndPlayReason == EEndPlayReason::Destroyed && GetGroupModules().Num() > 1)
    {
        RemoveGroupModule();
    }

    Super::EndPlay(EndPlayReason);
}

void ALBBuild_ModularLoadBalancer::Dismantle_Implementation()
{
    if (MyInputConnection && MyOutputConnection)
    {
        if (MyInputConnection->GetConnection() && MyOutputConnection->GetConnection())
        {
            auto inputLoc = MyInputConnection->GetConnection()->GetConnectorLocation();
            auto outputLoc = MyOutputConnection->GetConnection()->GetConnectorLocation();
            if ((inputLoc - outputLoc).IsNearlyZero(0.01))
            {
                auto belt1Conn = MyInputConnection->GetConnection();
                auto belt2Conn = MyOutputConnection->GetConnection();
                if (belt1Conn && belt2Conn)
                {
                    auto belt1 = Cast< AFGBuildableConveyorBelt>(belt1Conn->GetOuterBuildable());
                    auto belt2 = Cast< AFGBuildableConveyorBelt>(belt2Conn->GetOuterBuildable());
                    if (belt1 && belt2)
                    {
                        if (belt1->GetSpeed() == belt2->GetSpeed())
                        {
                            auto belt1CustomizationData = belt1->Execute_GetCustomizationData(belt1);
                            TArray< AFGBuildableConveyorBelt*> belts;
                            belts.Add(belt1);
                            belts.Add(belt2);
                            belt1Conn->ClearConnection();
                            belt2Conn->ClearConnection();
                            belt1Conn->SetConnection(belt2Conn);
                            auto newBelt = AFGBuildableConveyorBelt::Merge(belts);
                            if (newBelt && belt1CustomizationData.IsInitialized())
                            {
                                newBelt->Execute_SetCustomizationData(newBelt, belt1CustomizationData);
                            }
                        }
                        else
                        {
                            belt1Conn->ClearConnection();
                            belt2Conn->ClearConnection();
                            belt1Conn->SetConnection(belt2Conn);
                        }
                    }
                }
            }
        }
    }
    Super::Dismantle_Implementation();
}

void ALBBuild_ModularLoadBalancer::SetCustomizationData_Native(const FFactoryCustomizationData& customizationData, bool skipCombine)
{
    if (IsLeader())
    {
        SetCustomization(this, customizationData);
    }
    else if (isLookedAtForColor)
    {
        GroupLeader->SetCustomization(this, customizationData);
    }
    Super::SetCustomizationData_Native(customizationData, skipCombine); //Preview
}

void ALBBuild_ModularLoadBalancer::SetCustomization(ALBBuild_ModularLoadBalancer* instigator, const FFactoryCustomizationData& customizationData)
{
    if (IsLeader())
    {
        for (TWeakObjectPtr<ALBBuild_ModularLoadBalancer> module : mGroupModules)
        {
            if (module != nullptr && module != this && module != instigator)
            {
                module->SetCustomizationData_Native(customizationData);
            }
        }
        Super::SetCustomizationData_Native(customizationData);
    }
}

void ALBBuild_ModularLoadBalancer::StartIsAimedAtForColor_Implementation(AFGCharacterPlayer* byCharacter, bool isValid)
{
    Super::StartIsAimedAtForColor_Implementation(byCharacter, isValid);
    isLookedAtForColor = true;
}

void ALBBuild_ModularLoadBalancer::StopIsAimedAtForColor_Implementation(AFGCharacterPlayer* byCharacter)
{
    Super::StopIsAimedAtForColor_Implementation(byCharacter);
    isLookedAtForColor = false;
}

bool ALBBuild_ModularLoadBalancer::ShouldSave_Implementation() const
{
    return true;
}

void ALBBuild_ModularLoadBalancer::OnOutputItemRemoved(TSubclassOf<UFGItemDescriptor> itemClass, int32 numRemoved, UFGInventoryComponent* targetInventory)
{
    if(MyOutputConnection && GetBufferInventory())
    {
        int Index = MyOutputConnection->GetInventoryAccessIndex() + 1;

        const bool IsValidIndex = GetBufferInventory()->IsValidIndex(Index);
        if (IsValidIndex)
        {
            Index = GetBufferInventory()->GetFirstIndexWithItem(Index);
        }
        else
        {
            Index = GetBufferInventory()->GetFirstIndexWithItem();
        }

        if (Index == -1 && IsValidIndex)
        {
            Index = GetBufferInventory()->GetFirstIndexWithItem();
        }

        MyOutputConnection->SetInventoryAccessIndex(Index);
    }
}

void ALBBuild_ModularLoadBalancer::ApplyGroupModule()
{
    if (HasAuthority() && GroupLeader)
    {
        GroupLeader->mGroupModules.AddUnique(this);
        UpdateCache();
        if (IsFilterModule())
        {
            for (TSubclassOf<UFGItemDescriptor> Item : mFilteredItems)
            {
                GroupLeader->mNormalLoaderData.SetFilterItemForBalancer(this, Item);
            }
        }
        ForceNetUpdate();
    }
    else
    {
        UE_LOG(LoadBalancers_Log, Error, TEXT("Cannot Apply GroupLeader: Invalid"))
    }
}

void ALBBuild_ModularLoadBalancer::RemoveGroupModule()
{
    GroupLeader->mNormalLoaderData.RemoveBalancer(this);
    GroupLeader->mGroupModules.Remove(this);

    if (IsFilterModule())
    {
        for (TSubclassOf<UFGItemDescriptor> Item : mFilteredItems)
        {
            if (Item)
            {
                GroupLeader->mNormalLoaderData.RemoveBalancer(this, Item);
            }
        }
    }

    if (IsLeader())
    {
        for (ALBBuild_ModularLoadBalancer* LoadBalancer : GetGroupModules())
        {
            if (LoadBalancer != this && !LoadBalancer->IsPendingKillOrUnreachable())
            {
                LoadBalancer->mGroupModules = mGroupModules;
                LoadBalancer->mNormalLoaderData = mNormalLoaderData;
                LoadBalancer->ApplyLeader();
                break;
            }
        }
    }
    mGroupModules.Empty();
    mNormalLoaderData = FLBBalancerData();
    GroupLeader->UpdateCache();
    GroupLeader = nullptr;
}

void ALBBuild_ModularLoadBalancer::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    TSet<UActorComponent*> components = GetComponents();
    for (UActorComponent* ComponentsByClass : components)
    {
        if (UFGFactoryConnectionComponent* ConnectionComponent = Cast<UFGFactoryConnectionComponent>(ComponentsByClass))
        {
            if (ConnectionComponent->GetDirection() == EFactoryConnectionDirection::FCD_INPUT)
            {
                MyInputConnection = ConnectionComponent;
            }
            else if (ConnectionComponent->GetDirection() == EFactoryConnectionDirection::FCD_OUTPUT)
            {
                MyOutputConnection = ConnectionComponent;
            }
        }
    }
#if !WITH_EDITOR
    GetBufferInventory()->SetLocked(false);
#endif
}

void ALBBuild_ModularLoadBalancer::Factory_CollectInput_Implementation()
{
    Super::Factory_CollectInput_Implementation();

    // Only the leader with authority should collect input
    if (!IsLeader() || !HasAuthority())
    {
        return;
    }

    // Early return if no connected inputs exist
    if (GroupLeader->mNormalLoaderData.mConnectedInputs.Num() == 0)
    {
        return;
    }

    // Get and validate the current input index
    int32 Indexing = GroupLeader->mNormalLoaderData.mInputIndex;
    if (Indexing < 0 || !GroupLeader->mNormalLoaderData.mConnectedInputs.IsValidIndex(Indexing))
    {
        Indexing = 0;
    }

    // Get reference to connected inputs for efficiency
    const TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>>& ConnectedInputs = GroupLeader->mNormalLoaderData.mConnectedInputs;

    // Process input from connected balancers
    for (int unused = 0; unused < ConnectedInputs.Num(); unused++)
    {
        // Get the current input balancer at the current index
        TWeakObjectPtr<ALBBuild_ModularLoadBalancer> InputBalancer = 
            ConnectedInputs.IsValidIndex(Indexing) ? ConnectedInputs[Indexing] : nullptr;

        if (InputBalancer.IsValid() && !InputBalancer->IsPendingKill())
        {
            // Try to collect input from the current balancer
            if (CollectInput(InputBalancer.Get()))
            {
                // Successfully collected input, move to next balancer
                Indexing++;
                if (!ConnectedInputs.IsValidIndex(Indexing))
                {
                    Indexing = 0; // Wrap around to beginning
                }
            }
            else if (GroupLeader->mNormalLoaderData.HasAnyValidFilter())
            {
                // Try to collect from filter-specific inputs if normal collection failed
                TryCollectFromFilterInputs(ConnectedInputs);
            }
        }
    }

    // Update the input index for next time
    GroupLeader->mNormalLoaderData.mInputIndex = Indexing;
}

void ALBBuild_ModularLoadBalancer::TryCollectFromFilterInputs(const TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>>& ConnectedInputs)
{
    // Early exit if no filter inputs exist
    if (GroupLeader->mNormalLoaderData.mFilterInputMap.Num() == 0)
    {
        return;
    }

    // Try to collect from filter-specific inputs when normal collection failed
    for (TPair<TSubclassOf<UFGItemDescriptor>, int32>& FilterInputPair : GroupLeader->mNormalLoaderData.mFilterInputMap)
    {
        int32& FilterIndexing = FilterInputPair.Value;
        
        // Initialize filter indexing if needed
        if (FilterIndexing == -1)
        {
            FilterIndexing = 0;
        }

        // Get the filter balancer at the current index
        TWeakObjectPtr<ALBBuild_ModularLoadBalancer> FilterBalancer = 
            ConnectedInputs.IsValidIndex(FilterIndexing) ? ConnectedInputs[FilterIndexing] : nullptr;

        if (FilterBalancer.IsValid() && !FilterBalancer->IsPendingKill())
        {
            if (CollectInput(FilterBalancer.Get()))
            {
                // Success! Update index and exit
                FilterIndexing++;
                if (!ConnectedInputs.IsValidIndex(FilterIndexing))
                {
                    FilterIndexing = 0;
                }
                return; // Exit early on success
            }
        }
        
        // Always advance index (whether we succeeded or failed)
        FilterIndexing++;
        if (!ConnectedInputs.IsValidIndex(FilterIndexing))
        {
            FilterIndexing = 0;
        }
    }
}

bool ALBBuild_ModularLoadBalancer::Balancer_CollectInput()
{
    if (!GroupLeader)
    {
        return false;
    }

    for (TWeakObjectPtr<ALBBuild_ModularLoadBalancer> Balancer : GroupLeader->mNormalLoaderData.mConnectedInputs)
    {
        if (Balancer.IsValid())
        {
           return CollectInput(Balancer.Get());
        }
    }
    return false;
}

bool ALBBuild_ModularLoadBalancer::CollectInput(ALBBuild_ModularLoadBalancer* Module)
{
    if (!Module || Module->IsPendingKill())
    {
        return false;
    }

    UFGFactoryConnectionComponent* connection = Module->MyInputConnection;
    if (!connection || !GroupLeader || connection->IsPendingKill())
    {
        return false;
    }

    if (!connection->IsConnected())
    {
        return false;
    }

    // Try to peek at available items
    TArray<FInventoryItem> peeker;
    if (!connection->Factory_PeekOutput(peeker) || peeker.Num() == 0)
    {
        return true; // No items available, but connection is valid
    }

    FInventoryItem Item = peeker[0];
    float offset;

    // Try to send item to filter balancer first (highest priority)
    if (SendToFilterBalancer(Item))
    {
        return connection->Factory_GrabOutput(Item, offset);
    }

    // Try to send item to normal balancer second
    if (SendToNormalBalancer(Item))
    {
        return connection->Factory_GrabOutput(Item, offset);
    }

    // Try to send item to overflow balancer last (lowest priority)
    if (SendToOverflowBalancer(Item))
    {
        return connection->Factory_GrabOutput(Item, offset);
    }

    // No balancer could accept the item
    return false;
}
#pragma optimize("", on)
