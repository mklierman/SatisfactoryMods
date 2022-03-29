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
            GroupLeader->mFilteredModuleData.SetFilterItemForBalancer(this, ItemClass, GetBufferInventory()->GetAllowedItemOnIndex(0));
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
                ModularLoadBalancer->mFilteredModuleData = mFilteredModuleData;
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
    
    bool InputConnected = MyInputConnection->IsConnected();
    bool InputsContainThis = false;
    if (GroupLeader->mNormalLoaderData.mConnectedInputs.Num() > 0)
    {
        InputsContainThis = GroupLeader->mNormalLoaderData.mConnectedInputs.Contains(this);
    }

    if (MyInputConnection)
    {
        if (GroupLeader->mNormalLoaderData.mConnectedInputs.Num() > 0)
        {
            if (InputsContainThis && !InputConnected)
            {
                GroupLeader->mNormalLoaderData.mConnectedInputs.Remove(this);
            }
            else if (!InputsContainThis && InputConnected)
            {
                GroupLeader->mNormalLoaderData.mConnectedInputs.AddUnique(this);
            }
        }
        else if (InputConnected)
        {
            GroupLeader->mNormalLoaderData.mConnectedInputs.AddUnique(this);
        }
    }

    if (IsNormalModule())
    {
        bool OutputConnected = MyOutputConnection->IsConnected();
        bool OutputsContainThis = false;
        if (GroupLeader->mNormalLoaderData.mConnectedOutputs.Num() > 0)
        {
            OutputsContainThis = GroupLeader->mNormalLoaderData.mConnectedOutputs.Contains(this);
        }

        if (MyOutputConnection)
        {
            if (GroupLeader->mNormalLoaderData.mConnectedOutputs.Num() > 0)
            {
                if (OutputsContainThis && !OutputConnected)
                {
                    GroupLeader->mNormalLoaderData.mConnectedOutputs.Remove(this);
                }
                else if (!OutputsContainThis && OutputConnected)
                {
                    GroupLeader->mNormalLoaderData.mConnectedOutputs.AddUnique(this);
                }
            }
            else if (OutputConnected)
            {
                GroupLeader->mNormalLoaderData.mConnectedOutputs.AddUnique(this);
            }
        }
    }

    if (IsFilterModule())
    {
        bool FilterOutputConnected = MyOutputConnection->IsConnected();
        bool FilterOutputsContainThis = false;
        if (GroupLeader->mFilteredModuleData.mConnectedOutputs.Num() > 0)
        {
            FilterOutputsContainThis = GroupLeader->mFilteredModuleData.mConnectedOutputs.Contains(this);
        }

        if (GroupLeader->mFilteredModuleData.mConnectedOutputs.Num() > 0)
        {
            if (FilterOutputsContainThis && !FilterOutputConnected)
            {
                GroupLeader->mFilteredModuleData.RemoveBalancer(this, GetBufferInventory()->GetAllowedItemOnIndex(0));
            }
            else if (!FilterOutputsContainThis && FilterOutputConnected)
            {
                GroupLeader->mFilteredModuleData.mConnectedOutputs.AddUnique(this);
                GroupLeader->mFilteredModuleData.SetFilterItemForBalancer(this, GetBufferInventory()->GetAllowedItemOnIndex(0), GetBufferInventory()->GetAllowedItemOnIndex(0));
            }
        }
        else if (FilterOutputConnected)
        {
            GroupLeader->mFilteredModuleData.mConnectedOutputs.AddUnique(this);
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
            GroupLeader->mFilteredModuleData.RemoveBalancer(this, mFilteredItem);
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

TArray<UFGFactoryConnectionComponent*> ALBBuild_ModularLoadBalancer::GetConnections(EFactoryConnectionDirection Direction, bool Filtered) const
{
    if (!GroupLeader)
    {
        return {};
    }

    FLBBalancerData Data = Filtered ? GroupLeader->mFilteredModuleData : GroupLeader->mNormalLoaderData;

    TArray<UFGFactoryConnectionComponent*> Return;
    TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> Array = Direction == EFactoryConnectionDirection::FCD_INPUT ? Data.mConnectedInputs : Data.mConnectedOutputs;;

    for (TWeakObjectPtr<ALBBuild_ModularLoadBalancer> LoadBalancer : Array)
    {
        if (LoadBalancer.IsValid())
        {
            Return.Add(Direction == EFactoryConnectionDirection::FCD_INPUT ? LoadBalancer->MyInputConnection : LoadBalancer->MyOutputConnection);
        }
    }

    return Return;
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

    if (mNormalLoaderData.mConnectedInputs.Num() == 0)
    {
        return;
    }

    if (!mNormalLoaderData.mConnectedInputs.IsValidIndex(mNormalLoaderData.mInputIndex))
    {
        mNormalLoaderData.mInputIndex = 0;
    }

    // if still BREAK! something goes wrong here!
    if (!mNormalLoaderData.mConnectedInputs.IsValidIndex(mNormalLoaderData.mInputIndex))
    {
        UE_LOG(LoadBalancers_Log, Error, TEXT("mConnectedInputs has still a invalid Index!"));
        return;
    }

    if(CollectInput(mNormalLoaderData.mConnectedInputs[mNormalLoaderData.mInputIndex].Get()))
        mNormalLoaderData.mInputIndex++;
}

ALBBuild_ModularLoadBalancer* ALBBuild_ModularLoadBalancer::GetNextInputBalancer(FInventoryItem Item)
{
    int Index;
    ALBBuild_ModularLoadBalancer* Balancer = nullptr;
    if(HasFilterModule())
    {
        Index = mFilteredModuleData.GetOutputIndexFromItem(Item.ItemClass, true);
        //UE_LOG(LoadBalancers_Log, Warning, TEXT("HasFilterModule() %d"), Index);
        if(Index >= 0)
        {
            mFilteredModuleData.mFilterMap[Item.ItemClass].mOutputIndex++;
            const int NextIndex = mFilteredModuleData.mFilterMap[Item.ItemClass].mOutputIndex;
            if(!mFilteredModuleData.mFilterMap[Item.ItemClass].mBalancer.IsValidIndex(NextIndex))
            {
                mFilteredModuleData.mFilterMap[Item.ItemClass].mOutputIndex = 0;
            }
                
            if(mFilteredModuleData.mFilterMap[Item.ItemClass].mBalancer.IsValidIndex(Index))
            {
                //UE_LOG(LoadBalancers_Log, Warning, TEXT("IsValidIndex"));
                if(mFilteredModuleData.mFilterMap[Item.ItemClass].mBalancer[Index].IsValid())
                    Balancer = mFilteredModuleData.mFilterMap[Item.ItemClass].mBalancer[Index].Get();
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

    Index = mNormalLoaderData.GetOutputIndexFromItem(Item.ItemClass);
    //UE_LOG(LoadBalancers_Log, Warning, TEXT("!HasFilterModule() %d"), Index);
    if(Index >= 0)
    {
        mNormalLoaderData.mOutputIndex[Item.ItemClass]++;
        const int NextIndex = mNormalLoaderData.mOutputIndex[Item.ItemClass];
        if(!mNormalLoaderData.mConnectedOutputs.IsValidIndex(NextIndex))
        {
            mNormalLoaderData.mOutputIndex[Item.ItemClass] = 0;
        }
    
        if(mNormalLoaderData.mConnectedOutputs.IsValidIndex(Index))
        {
            //UE_LOG(LoadBalancers_Log, Warning, TEXT("IsValidIndex"));
            if(mNormalLoaderData.mConnectedOutputs[Index].IsValid())
                Balancer = mNormalLoaderData.mConnectedOutputs[Index].Get();
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
    }
    return false;
}
