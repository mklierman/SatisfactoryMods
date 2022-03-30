#include "LBBuild_ModularLoadBalancer.h"

#include "FGColoredInstanceMeshProxy.h"
#include "FGFactoryConnectionComponent.h"
#include "FGInventoryLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "FGOutlineComponent.h"
#include "LBDefaultRCO.h"
#include "LoadBalancersModule.h"

DEFINE_LOG_CATEGORY(LoadBalancers_Log);
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

        if(IsFilterModule())
            SetFilteredItem(mFilteredItem);
        ApplyGroupModule();
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

    if (MyOutputConnection && IsNormalModule())
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
        if (IsOverflowModule())
        {
            GroupLeader->mOverflowModule = this;
        }

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

    if(CollectInput(GroupLeader->mNormalLoaderData.mConnectedInputs[CachedIndex].Get()))
        GroupLeader->mNormalLoaderData.mInputIndex++;
}

ALBBuild_ModularLoadBalancer* ALBBuild_ModularLoadBalancer::GetNextInputBalancer(FInventoryItem Item)
{
    if(!GroupLeader)
        return nullptr;

    int Index;
    ALBBuild_ModularLoadBalancer* Balancer = nullptr;
    if(HasFilterModule() && GroupLeader->mNormalLoaderData.HasItemFilterBalancer(Item.ItemClass))
    {
        Index = GroupLeader->mNormalLoaderData.GetOutputIndexFromItem(Item.ItemClass, true);
        //UE_LOG(LoadBalancers_Log, Warning, TEXT("HasFilterModule() %d"), Index);
        if(Index >= 0)
        {
            GroupLeader->mNormalLoaderData.mFilterMap[Item.ItemClass].mOutputIndex++;
            const int NextIndex = GroupLeader->mNormalLoaderData.mFilterMap[Item.ItemClass].mOutputIndex;
            if(!GroupLeader->mNormalLoaderData.mFilterMap[Item.ItemClass].mBalancer.IsValidIndex(NextIndex))
            {
                GroupLeader->mNormalLoaderData.mFilterMap[Item.ItemClass].mOutputIndex = 0;
            }

            if(GroupLeader->mNormalLoaderData.mFilterMap[Item.ItemClass].mBalancer.IsValidIndex(Index))
            {
                //UE_LOG(LoadBalancers_Log, Warning, TEXT("IsValidIndex"));
                if(GroupLeader->mNormalLoaderData.mFilterMap[Item.ItemClass].mBalancer[Index].IsValid())
                    Balancer = GroupLeader->mNormalLoaderData.mFilterMap[Item.ItemClass].mBalancer[Index].Get();
            }
            else
            {
                return nullptr;
            }
        }
    }

    if(Balancer)
        if(Balancer->GetBufferInventory()->GetNumItems(Item.ItemClass) < 1 && Balancer->GetBufferInventory()->HasEnoughSpaceForItem(Item))
        {
            return Balancer;
        }

    Index = GroupLeader->mNormalLoaderData.GetOutputIndexFromItem(Item.ItemClass);
    //UE_LOG(LoadBalancers_Log, Warning, TEXT("!HasFilterModule() %d"), Index);
    if(Index >= 0)
    {
        GroupLeader->mNormalLoaderData.mOutputIndex[Item.ItemClass]++;
        const int NextIndex = GroupLeader->mNormalLoaderData.mOutputIndex[Item.ItemClass];
        if(!GroupLeader->mNormalLoaderData.mConnectedOutputs.IsValidIndex(NextIndex))
        {
            GroupLeader->mNormalLoaderData.mOutputIndex[Item.ItemClass] = 0;
        }

        if(GroupLeader->mNormalLoaderData.mConnectedOutputs.IsValidIndex(Index))
        {
            //UE_LOG(LoadBalancers_Log, Warning, TEXT("IsValidIndex"));
            if(GroupLeader->mNormalLoaderData.mConnectedOutputs[Index].IsValid())
                Balancer = GroupLeader->mNormalLoaderData.mConnectedOutputs[Index].Get();
        }
        else
        {
            return nullptr;
        }
    }

    if(Balancer)
        if(Balancer->GetBufferInventory()->GetNumItems(Item.ItemClass) < 1 && Balancer->GetBufferInventory()->HasEnoughSpaceForItem(Item))
        {
            return Balancer;
        }

    return nullptr;
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

            if(ALBBuild_ModularLoadBalancer* Balancer = GetNextInputBalancer(Item))
            {
                //UE_LOG(LoadBalancers_Log, Warning, TEXT("Factory_GrabOutput, %d"), Balancer->IsFilterModule());
                if (connection->Factory_GrabOutput(Item, offset))
                {
                    //UE_LOG(LoadBalancers_Log, Warning, TEXT("AddItem"));
                    Balancer->GetBufferInventory()->AddItem(Item);
                    return true;
                }
            }

            if (HasOverflowModule())
            {
                ALBBuild_ModularLoadBalancer* OverflowModule = GetOverflowLoader();
                if (OverflowModule->GetBufferInventory() &&
                    OverflowModule->GetBufferInventory()->HasEnoughSpaceForItem(Item)
                    )
                {
                    if(connection->Factory_GrabOutput(Item, offset))
                    {
                        OverflowModule->GetBufferInventory()->AddItem(Item);
                        return true;
                    }
                }
            }
        }
        else
            return true;
    }
    return false;
}
