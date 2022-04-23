#include "LBBuild_ModularLoadBalancer.h"

#include "FGColoredInstanceMeshProxy.h"
#include "FGFactoryConnectionComponent.h"
#include "FGInventoryLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "FGOutlineComponent.h"
#include "LBDefaultRCO.h"
#include "LoadBalancersModule.h"

DEFINE_LOG_CATEGORY(LoadBalancers_Log);

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

int FLBBalancerData::GetOutputIndexFromItem(TSubclassOf<UFGItemDescriptor> Item, bool IsFilter)
{
    if(IsFilter)
    {
        if(HasItemFilterBalancer(Item))
        {
            return mFilterMap[Item].mOutputIndex;
        }
        return -1;
    }

    if(mOutputIndex.Contains(Item))
    {
        return mOutputIndex[Item];
    }
		
    mOutputIndex.Add(Item, 0);
    return 0;
}

void FLBBalancerData::SetFilterItemForBalancer(ALBBuild_ModularLoadBalancer* Balancer,
                                               TSubclassOf<UFGItemDescriptor> Item, TSubclassOf<UFGItemDescriptor> OldItem)
{
    if(HasItemFilterBalancer(OldItem))
    {
        RemoveBalancer(Balancer, OldItem);
    }
		
    if(HasItemFilterBalancer(Item))
    {
        mFilterMap[Item].mBalancer.AddUnique(Balancer);
        if(mFilterMap[Item].mOutputIndex == -1)
            mFilterMap[Item] = 0;
    }
    else
    {
        mFilterMap.Add(Item, FLBBalancerData_Filters(Balancer));
        if(mFilterMap[Item].mOutputIndex == -1)
            mFilterMap[Item] = 0;
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

        if(Balancer)
        {
            if(mConnectedOutputs.Contains(Balancer))
                mConnectedOutputs.Remove(Balancer);
        }
    }
}

bool FLBBalancerData::HasItemFilterBalancer(TSubclassOf<UFGItemDescriptor> Item) const
{
    if(Item)
        return mFilterMap.Contains(Item);
    return false;
}

TArray<ALBBuild_ModularLoadBalancer*> FLBBalancerData::GetBalancerForFilters(TSubclassOf<UFGItemDescriptor> Item)
{
    TArray<ALBBuild_ModularLoadBalancer*> Balancers = {};

    if(HasItemFilterBalancer(Item))
    {
        for (TWeakObjectPtr<ALBBuild_ModularLoadBalancer> Balancer : mFilterMap[Item].mBalancer)
        {
            if(Balancer.IsValid())
            {
                Balancers.Add(Balancer.Get());
            }
        }
    }

    return Balancers;
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
    DOREPLIFETIME(ALBBuild_ModularLoadBalancer, mFilteredItem);
}

void ALBBuild_ModularLoadBalancer::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        if(!GroupLeader)
            GroupLeader = this;

        if (GetBufferInventory() && MyOutputConnection)
        {
            GetBufferInventory()->OnItemRemovedDelegate.AddDynamic(this, &ALBBuild_ModularLoadBalancer::OnOutputItemRemoved);
            MyOutputConnection->SetInventory(GetBufferInventory());
            GetBufferInventory()->Resize(20);

            for (int i = 0; i < GetBufferInventory()->GetSizeLinear(); ++i)
            {
                if (GetBufferInventory()->IsValidIndex(i))
                {
                    GetBufferInventory()->AddArbitrarySlotSize(i, 1);

                    if (IsFilterModule())
                    {
                        GetBufferInventory()->SetAllowedItemOnIndex(i, mFilteredItem);
                    }
                }
            }
        }

        ApplyGroupModule();
        if(IsFilterModule())
            SetFilteredItem(mFilteredItem);
    }
}

void ALBBuild_ModularLoadBalancer::SetFilteredItem(TSubclassOf<UFGItemDescriptor> ItemClass)
{
    if (HasAuthority())
    {
        if (IsFilterModule() && GetBufferInventory())
        {
            GroupLeader->mNormalLoaderData.SetFilterItemForBalancer(this, ItemClass, GetBufferInventory()->GetAllowedItemOnIndex(0));
            for (int i = 0; i < GetBufferInventory()->GetSizeLinear(); ++i)
            {
                if (GetBufferInventory()->IsValidIndex(i))
                {
                    GetBufferInventory()->SetAllowedItemOnIndex(i, ItemClass);
                }
            }
            mFilteredItem = GetBufferInventory()->GetAllowedItemOnIndex(0);
        }
    }
    else if (ULBDefaultRCO* RCO = ULBDefaultRCO::Get(GetWorld()))
    {
        RCO->Server_SetFilteredItem(this, ItemClass);
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

bool ALBBuild_ModularLoadBalancer::SendToOverflowBalancer(FInventoryItem Item)
{
    if(HasOverflowModule())
        for (TWeakObjectPtr<ALBBuild_ModularLoadBalancer> OverflowBalancer : mNormalLoaderData.mOverflowBalancer)
        {
            if(OverflowBalancer.IsValid())
            {
                if(OverflowBalancer->GetBufferInventory())
                {
                    if(OverflowBalancer->GetBufferInventory()->HasEnoughSpaceForItem(Item))
                    {
                        OverflowBalancer->GetBufferInventory()->AddItem(Item);
                        mNormalLoaderData.mOverflowBalancer.Remove(OverflowBalancer);
                        mNormalLoaderData.mOverflowBalancer.Add(OverflowBalancer);
                        return true;
                    }
                }
            }
            mNormalLoaderData.mOverflowBalancer.Remove(OverflowBalancer);
            mNormalLoaderData.mOverflowBalancer.Add(OverflowBalancer);
        }
    return false;
}

bool ALBBuild_ModularLoadBalancer::SendToFilterBalancer(FInventoryItem Item)
{
    if(HasFilterModule())
        if(mNormalLoaderData.mFilterMap.Contains(Item.ItemClass))
            if(mNormalLoaderData.mFilterMap[Item.ItemClass].mBalancer.Num() > 0)
                for (TWeakObjectPtr<ALBBuild_ModularLoadBalancer> OverflowBalancer : mNormalLoaderData.mFilterMap[Item.ItemClass].mBalancer)
                {
                    if(OverflowBalancer.IsValid())
                    {
                        if(OverflowBalancer->GetBufferInventory())
                        {
                            if(OverflowBalancer->GetBufferInventory()->HasEnoughSpaceForItem(Item))
                            {
                                OverflowBalancer->GetBufferInventory()->AddItem(Item);
                                mNormalLoaderData.mFilterMap[Item.ItemClass].mBalancer.Remove(OverflowBalancer);
                                mNormalLoaderData.mFilterMap[Item.ItemClass].mBalancer.Add(OverflowBalancer);
                                return true;
                            }
                        }
                    }
                    mNormalLoaderData.mFilterMap[Item.ItemClass].mBalancer.Remove(OverflowBalancer);
                    mNormalLoaderData.mFilterMap[Item.ItemClass].mBalancer.Add(OverflowBalancer);
                }
    return false;
}

bool ALBBuild_ModularLoadBalancer::SendToNormalBalancer(FInventoryItem Item)
{
    if(mNormalLoaderData.mConnectedOutputs.Num() > 0)
        for (TWeakObjectPtr<ALBBuild_ModularLoadBalancer> OverflowBalancer : mNormalLoaderData.mConnectedOutputs)
        {
            if(OverflowBalancer.IsValid())
            {
                if(OverflowBalancer->GetBufferInventory())
                {
                    if(OverflowBalancer->GetBufferInventory()->HasEnoughSpaceForItem(Item))
                    {
                        OverflowBalancer->GetBufferInventory()->AddItem(Item);
                        mNormalLoaderData.mConnectedOutputs.Remove(OverflowBalancer);
                        mNormalLoaderData.mConnectedOutputs.Add(OverflowBalancer);
                        return true;
                    }
                }
            }
            mNormalLoaderData.mConnectedOutputs.Remove(OverflowBalancer);
            mNormalLoaderData.mConnectedOutputs.Add(OverflowBalancer);
        }
    return false;
}

void ALBBuild_ModularLoadBalancer::Factory_Tick(float dt)
{
    Super::Factory_Tick(dt);

    if (IsLeader() && HasAuthority())
    {
        if (IsFilterModule() && GetBufferInventory())
        {
            if (mFilteredItem != GetBufferInventory()->GetAllowedItemOnIndex(0))
            {
                SetFilteredItem(mFilteredItem);
            }
        }
    }

    if (MyOutputConnection && HasAuthority())
    {
        mTimer += dt;
        if (mTimer >= 1.f)
        {
            mTimer -= 1.f;
            UpdateCache();
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
        if(IsFilterModule() && GroupLeader)
        {
            GroupLeader->mNormalLoaderData.RemoveBalancer(this, mFilteredItem);
        }

        if (IsLeader() && GetGroupModules().Num() > 1)
        {
            for (ALBBuild_ModularLoadBalancer* LoadBalancer : GetGroupModules())
            {
                if (LoadBalancer != this)
                {
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
        return;
    
    int CachedIndex = GroupLeader->mNormalLoaderData.mInputIndex;

    if (!GroupLeader->mNormalLoaderData.mConnectedInputs.IsValidIndex(CachedIndex))
    {
        GroupLeader->mNormalLoaderData.mInputIndex = 0;
        CachedIndex = 0;
    }

    // if still BREAK! something goes wrong here!
    if (!GroupLeader->mNormalLoaderData.mConnectedInputs.IsValidIndex(CachedIndex))
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
