#include "LBBuild_ModularLoadBalancer.h"

#include "FGColoredInstanceMeshProxy.h"
#include "FGFactoryConnectionComponent.h"
#include "FGInventoryLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "FGOutlineComponent.h"
#include "LBDefaultRCO.h"
#include "LoadBalancersModule.h"

DEFINE_LOG_CATEGORY(LoadBalancers_Log);
DEFINE_LOG_CATEGORY(LogGame);

void FLBBalancerData::GetInputBalancers(TArray<ALBBuild_ModularLoadBalancer*>& Out)
{
    for (TWeakObjectPtr<ALBBuild_ModularLoadBalancer> Balancer : mConnectedInputs)
        if(Balancer.IsValid())
            Out.AddUnique(Balancer.Get());
}

bool FLBBalancerData::HasAnyValidFilter() const
{
    if(mFilterMap.Num() >= 0)
    {
        TArray<TSubclassOf<UFGItemDescriptor>> Keys;
        mFilterMap.GenerateKeyArray(Keys);
        return !(mFilterMap.Num() == 1 && Keys.Contains(UFGNoneDescriptor::StaticClass()));
    }
    return true;
}

bool FLBBalancerData::HasAnyValidOverflow() const
{
    if(mOverflowBalancer.Num() >= 0)
    {
        for (TWeakObjectPtr<ALBBuild_ModularLoadBalancer> Overflower : mOverflowBalancer)
            if(Overflower.IsValid())
                return true;
    }
    return true;
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
}

void FLBBalancerData::RemoveBalancer(ALBBuild_ModularLoadBalancer* Balancer, TSubclassOf<UFGItemDescriptor> OldItem)
{
    if(HasItemFilterBalancer(OldItem))
    {
        if(mFilterMap[OldItem].mBalancer.Contains(Balancer))
            mFilterMap[OldItem].mBalancer.Remove(Balancer);
        if(mFilterMap[OldItem].mBalancer.Num() == 0)
            mFilterMap.Remove(OldItem);
    }

    if(Balancer)
    {
        if(mConnectedOutputs.Contains(Balancer))
            mConnectedOutputs.Remove(Balancer);

        if(mConnectedInputs.Contains(Balancer))
            mConnectedInputs.Remove(Balancer);

        if(Balancer->IsOverflowModule())
            if(mOverflowBalancer.Contains(Balancer))
                mOverflowBalancer.Remove(Balancer);
    }
}

bool FLBBalancerData::HasItemFilterBalancer(TSubclassOf<UFGItemDescriptor> Item) const
{
    if(Item)
        return mFilterMap.Contains(Item);
    return false;
}

ALBBuild_ModularLoadBalancer::ALBBuild_ModularLoadBalancer()
{
    this->mInventorySizeX = 2;
    this->mInventorySizeY = 2;
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

    if (HasAuthority())
    {
        if(!GroupLeader)
            GroupLeader = this;

        if(mFilteredItem)
        {
            mFilteredItems.SetNum(1);
            mFilteredItems[0] = mFilteredItem;
            mFilteredItem = nullptr;
        }

        SetupInventory();

        if(MyOutputConnection && GetBufferInventory())
        {
            int32 Index = MyOutputConnection->GetInventoryAccessIndex();
            if(GetBufferInventory()->IsIndexEmpty(Index))
                MyOutputConnection->SetInventoryAccessIndex(-1);
        }

        ApplyGroupModule();
    }
}

void ALBBuild_ModularLoadBalancer::SetupInventory()
{
    if (GetBufferInventory())
    {
        if(!IsFilterModule())
        {
            if(MyOutputConnection)
                MyOutputConnection->SetInventory(GetBufferInventory());

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
                MyOutputConnection->SetInventory(GetBufferInventory());

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
    }
}

void ALBBuild_ModularLoadBalancer::SetFilteredItem(TSubclassOf<UFGItemDescriptor> ItemClass)
{
    if(!ItemClass || mFilteredItems.Contains(ItemClass))
        return;

    if(mLoaderType == ELoaderType::Programmable)
        if(mFilteredItems.Num() >= mMaxFilterableItems)
            return;

    if (HasAuthority())
    {
        if (IsFilterModule() && GetBufferInventory())
        {
            if(GroupLeader)
            {
                if(mLoaderType == ELoaderType::Filter)
                {
                    TSubclassOf<UFGItemDescriptor> OldItem = mFilteredItems[0];
                    mFilteredItems[0] = ItemClass;
                    GroupLeader->mNormalLoaderData.RemoveBalancer(this, OldItem);
                }
                else
                {
                    if(mFilteredItems[0] == UFGNoneDescriptor::StaticClass())
                        mFilteredItems[0] = ItemClass;
                    else
                     mFilteredItems.Add(ItemClass);
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
        return;

    if (HasAuthority())
    {
        if (IsFilterModule() && GetBufferInventory())
        {
            mFilteredItems.Remove(ItemClass);
            if(GroupLeader)
                GroupLeader->mNormalLoaderData.RemoveBalancer(this, ItemClass);

            if(mFilteredItems.Num() == 0)
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
    if(!GroupLeader)
        return false;

    if(HasOverflowModule())
    {
        FLBBalancerIndexing* Indexing = GroupLeader->mNormalLoaderData.mIndexMapping.Find(Item.ItemClass);
        if(!Indexing)
        {
            GroupLeader->mNormalLoaderData.mIndexMapping.Add(Item.ItemClass, FLBBalancerIndexing());
            Indexing = GroupLeader->mNormalLoaderData.mIndexMapping.Find(Item.ItemClass);
        }

        if(Indexing)
        {
            if(Indexing->mOverflowIndex == -1)
                Indexing->mOverflowIndex = 0;
        }
        else
            return false;

        for(int i = 0; i < GroupLeader->mNormalLoaderData.mOverflowBalancer.Num(); i++)
        {
            TWeakObjectPtr<ALBBuild_ModularLoadBalancer> OverflowBalancer = GroupLeader->mNormalLoaderData.mOverflowBalancer.IsValidIndex(Indexing->mOverflowIndex) ? GroupLeader->mNormalLoaderData.mOverflowBalancer[Indexing->mOverflowIndex] : nullptr;
            Indexing->mOverflowIndex++;

            if(!GroupLeader->mNormalLoaderData.mOverflowBalancer.IsValidIndex(Indexing->mOverflowIndex))
                Indexing->mOverflowIndex = 0;

            if(OverflowBalancer.IsValid())
            {
                if(OverflowBalancer->GetBufferInventory())
                {
                    if(OverflowBalancer->GetBufferInventory()->GetNumItems(Item.ItemClass) < mOverwriteSlotSize)
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

bool ALBBuild_ModularLoadBalancer::SendToFilterBalancer(FInventoryItem Item) const
{
    if(!GroupLeader)
        return false;

    if(HasFilterModule())
        if(GroupLeader->mNormalLoaderData.HasItemFilterBalancer(Item.ItemClass))
        {
            FLBBalancerIndexing* Indexing = GroupLeader->mNormalLoaderData.mIndexMapping.Find(Item.ItemClass);
            if(!Indexing)
            {
                GroupLeader->mNormalLoaderData.mIndexMapping.Add(Item.ItemClass, FLBBalancerIndexing());
                Indexing = GroupLeader->mNormalLoaderData.mIndexMapping.Find(Item.ItemClass);
            }

            if(Indexing)
            {
                if(Indexing->mFilterIndex == -1)
                    Indexing->mFilterIndex = 0;
            }
            else
            {
                return false;
            }

            for(int i = 0; i < GroupLeader->mNormalLoaderData.mFilterMap[Item.ItemClass].mBalancer.Num(); i++)
            {
                TWeakObjectPtr<ALBBuild_ModularLoadBalancer> OverflowBalancer = GroupLeader->mNormalLoaderData.mFilterMap[Item.ItemClass].mBalancer.IsValidIndex(Indexing->mFilterIndex) ? GroupLeader->mNormalLoaderData.mFilterMap[Item.ItemClass].mBalancer[Indexing->mFilterIndex] : nullptr;
                Indexing->mFilterIndex++;

                if(!GroupLeader->mNormalLoaderData.mFilterMap[Item.ItemClass].mBalancer.IsValidIndex(Indexing->mFilterIndex))
                    Indexing->mFilterIndex = 0;

                if(OverflowBalancer.IsValid())
                {
                    if(OverflowBalancer->GetBufferInventory())
                    {
                        if(OverflowBalancer->GetBufferInventory()->GetNumItems(Item.ItemClass) < mOverwriteSlotSize)
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

bool ALBBuild_ModularLoadBalancer::SendToNormalBalancer(FInventoryItem Item) const
{
    if(!GroupLeader)
        return false;

    if(GroupLeader->mNormalLoaderData.mConnectedOutputs.Num() > 0)
    {
        FLBBalancerIndexing* Indexing = GroupLeader->mNormalLoaderData.mIndexMapping.Find(Item.ItemClass);
        if(!Indexing)
        {
            GroupLeader->mNormalLoaderData.mIndexMapping.Add(Item.ItemClass, FLBBalancerIndexing());
            Indexing = GroupLeader->mNormalLoaderData.mIndexMapping.Find(Item.ItemClass);
        }

        if(Indexing)
        {
            if(Indexing->mNormalIndex == -1)
                Indexing->mNormalIndex = 0;
        }
        else
            return false;

        for(int i = 0; i < GroupLeader->mNormalLoaderData.mConnectedOutputs.Num(); i++)
        {
            TWeakObjectPtr<ALBBuild_ModularLoadBalancer> OverflowBalancer = GroupLeader->mNormalLoaderData.mConnectedOutputs.IsValidIndex(Indexing->mNormalIndex) ? GroupLeader->mNormalLoaderData.mConnectedOutputs[Indexing->mNormalIndex] : nullptr;
            Indexing->mNormalIndex++;

            if(!GroupLeader->mNormalLoaderData.mConnectedOutputs.IsValidIndex(Indexing->mNormalIndex))
                Indexing->mNormalIndex = 0;

            if(OverflowBalancer.IsValid())
            {
                if(OverflowBalancer->GetBufferInventory())
                {
                    if(OverflowBalancer->GetBufferInventory()->GetNumItems(Item.ItemClass) < mOverwriteSlotSize)
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

void ALBBuild_ModularLoadBalancer::Factory_Tick(float dt)
{
    Super::Factory_Tick(dt);

    if (HasAuthority())
    {
        if(GetBufferInventory()->GetSizeLinear() != mSlotsInBuffer)
            SetupInventory();

        if(IsLeader())
            if (IsFilterModule() && GetBufferInventory())
            {
                if (mFilteredItem != GetBufferInventory()->GetAllowedItemOnIndex(0))
                {
                    SetFilteredItem(mFilteredItem);
                }
            }

        if (MyOutputConnection || MyInputConnection)
        {
            mTimer += dt;
            if (mTimer >= 1.f)
            {
                mTimer -= 1.f;
                UpdateCache();
            }
        }

        if(MyOutputConnection && GetBufferInventory())
            if(!GetBufferInventory()->IsValidIndex(MyOutputConnection->GetInventoryAccessIndex()) && MyOutputConnection->GetInventoryAccessIndex() != -1)
                MyOutputConnection->SetInventoryAccessIndex(-1);
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
        if (GroupLeader->mNormalLoaderData.mConnectedInputs.Num() > 0)
        {
            if (GroupLeader->mNormalLoaderData.mConnectedInputs.Contains(this) && !MyInputConnection->IsConnected())
            {
                GroupLeader->mNormalLoaderData.mConnectedInputs.Remove(this);
            }
            else if (!GroupLeader->mNormalLoaderData.mConnectedInputs.Contains(this) && MyInputConnection->IsConnected())
            {
                GroupLeader->mNormalLoaderData.mConnectedInputs.AddUnique(this);
            }
        }
        else if (MyInputConnection->IsConnected())
        {
            GroupLeader->mNormalLoaderData.mConnectedInputs.AddUnique(this);
        }
    }

    if (MyOutputConnection && IsOverflowModule())
    {
        if (GroupLeader->mNormalLoaderData.mOverflowBalancer.Num() > 0)
        {
            if (GroupLeader->mNormalLoaderData.mOverflowBalancer.Contains(this) && !MyOutputConnection->IsConnected())
            {
                GroupLeader->mNormalLoaderData.mOverflowBalancer.Remove(this);
            }
            else if (!GroupLeader->mNormalLoaderData.mOverflowBalancer.Contains(this) && MyOutputConnection->IsConnected())
            {
                GroupLeader->mNormalLoaderData.mOverflowBalancer.AddUnique(this);
            }
        }
        else if (MyOutputConnection->IsConnected())
        {
            GroupLeader->mNormalLoaderData.mOverflowBalancer.AddUnique(this);
        }
    }

    else if (MyOutputConnection && IsNormalModule())
    {
        if (GroupLeader->mNormalLoaderData.mConnectedOutputs.Num() > 0)
        {
            if (GroupLeader->mNormalLoaderData.mConnectedOutputs.Contains(this) && !MyOutputConnection->IsConnected())
            {
                GroupLeader->mNormalLoaderData.mConnectedOutputs.Remove(this);
            }
            else if (!GroupLeader->mNormalLoaderData.mConnectedOutputs.Contains(this) && MyOutputConnection->IsConnected())
            {
                GroupLeader->mNormalLoaderData.mConnectedOutputs.AddUnique(this);
            }
        }
        else if (MyOutputConnection->IsConnected())
        {
            GroupLeader->mNormalLoaderData.mConnectedOutputs.AddUnique(this);
        }
    }
}

void ALBBuild_ModularLoadBalancer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Apply new Leader to the group if the group has more as 1 module (single loader)
    // EndPlay is here a better use. We should do this better only if it's Destroyed on EndPlay (is also in no case to late to can be invalid)
    if (EndPlayReason == EEndPlayReason::Destroyed)
    {
        GroupLeader->mNormalLoaderData.RemoveBalancer(this);
        GroupLeader->mGroupModules.Remove(this);

        if(IsFilterModule())
            for (TSubclassOf<UFGItemDescriptor> Item : mFilteredItems)
                GroupLeader->mNormalLoaderData.RemoveBalancer(this, Item);

        if (IsLeader() && GetGroupModules().Num() > 1)
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
    }

   Super::EndPlay(EndPlayReason);
}

void ALBBuild_ModularLoadBalancer::OnOutputItemRemoved(TSubclassOf<UFGItemDescriptor> itemClass, int32 numRemoved)
{
    if(MyOutputConnection && GetBufferInventory())
    {
        int Index = MyOutputConnection->GetInventoryAccessIndex() + 1;

        const bool IsValidIndex = GetBufferInventory()->IsValidIndex(Index);
        if(IsValidIndex)
            Index = GetBufferInventory()->GetFirstIndexWithItem(Index);
        else
            Index = GetBufferInventory()->GetFirstIndexWithItem();

        if(Index == -1 && IsValidIndex)
            Index = GetBufferInventory()->GetFirstIndexWithItem();

        MyOutputConnection->SetInventoryAccessIndex(Index);
    }
}

void ALBBuild_ModularLoadBalancer::ApplyGroupModule()
{
    if (HasAuthority() && GroupLeader)
    {
        GroupLeader->mGroupModules.AddUnique(this);
        UpdateCache();
        if(IsFilterModule())
            for (TSubclassOf<UFGItemDescriptor> Item : mFilteredItems)
                GroupLeader->mNormalLoaderData.SetFilterItemForBalancer(this, Item);
        ForceNetUpdate();
    }
    else
    {
        UE_LOG(LoadBalancers_Log, Error, TEXT("Cannot Apply GroupLeader: Invalid"))
    }
}

void ALBBuild_ModularLoadBalancer::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    for (UActorComponent* ComponentsByClass : GetComponentsByClass(UFGFactoryConnectionComponent::StaticClass()))
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
}

void ALBBuild_ModularLoadBalancer::Factory_CollectInput_Implementation()
{
    Super::Factory_CollectInput_Implementation();

    if (!IsLeader() || !HasAuthority())
    {
        return;
    }

    TArray<ALBBuild_ModularLoadBalancer*> Balancers;
    GroupLeader->mNormalLoaderData.GetInputBalancers(Balancers);

    if (Balancers.Num() == 0)
    {
        return;
    }

    for (ALBBuild_ModularLoadBalancer* Balancer : Balancers)
    {
        if(!Balancer->IsPendingKill())
        {
            Balancer->Balancer_CollectInput();
        }
    }
}

void ALBBuild_ModularLoadBalancer::Balancer_CollectInput()
{
    if(!GroupLeader)
    {
        return;
    }

    for (TWeakObjectPtr<ALBBuild_ModularLoadBalancer> Balancer : GroupLeader->mNormalLoaderData.mConnectedInputs)
    {
        if(Balancer.IsValid())
        {
            CollectInput(Balancer.Get());
        }
    }
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

    if (connection->IsConnected())
    {
        TArray<FInventoryItem> peeker;
        if (connection->Factory_PeekOutput(peeker))
        {
            FInventoryItem Item = peeker[0];
            float offset;

            if(SendToFilterBalancer(Item))
                return connection->Factory_GrabOutput(Item, offset);

            if (SendToNormalBalancer(Item))
                return connection->Factory_GrabOutput(Item, offset);

            if (SendToOverflowBalancer(Item))
                return connection->Factory_GrabOutput(Item, offset);
        }
        else
            return true;
    }
    return false;
}
