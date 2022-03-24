


#include "LBBuild_ModularLoadBalancer.h"

#include "FGColoredInstanceMeshProxy.h"
#include "FGFactoryConnectionComponent.h"
#include "FGInventoryLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "FGOutlineComponent.h"
#include "LoadBalancersModule.h"

DEFINE_LOG_CATEGORY(LoadBalancers_Log);
ALBBuild_ModularLoadBalancer::ALBBuild_ModularLoadBalancer()
{
    this->mInventorySizeX = 2;
    this->mInventorySizeY = 2;
    mOutputInventory = CreateDefaultSubobject<UFGInventoryComponent>(TEXT("OutputInventory"));
}

void ALBBuild_ModularLoadBalancer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALBBuild_ModularLoadBalancer, mGroupModules);
    DOREPLIFETIME(ALBBuild_ModularLoadBalancer, GroupLeader);
}

void ALBBuild_ModularLoadBalancer::BeginPlay()
{
    Super::BeginPlay();

    if(HasAuthority())
    {
        if(!mOutputInventory)
            mOutputInventory = UFGInventoryLibrary::CreateInventoryComponent(this, TEXT("OutputInventory"));

        if(GetBufferInventory() && MyOutputConnection)
        {
            MyOutputConnection->SetInventory(GetBufferInventory());
            MyOutputConnection->SetInventoryAccessIndex(0);
            GetBufferInventory()->Resize(1);
            GetBufferInventory()->AddArbitrarySlotSize(0, 5);
        }

        if(mOutputInventory)
        {
            mOutputInventory->Resize(10);
            for(int i = 0; i < 10; ++i)
                mOutputInventory->AddArbitrarySlotSize(i, 5);
        }
        
        ApplyGroupModule();
    }

}

void ALBBuild_ModularLoadBalancer::ApplyLeader()
{
    if(HasAuthority())
    {
        GroupLeader = this;

        for (ALBBuild_ModularLoadBalancer* ModularLoadBalancer : GetGroupModules())
            if(ModularLoadBalancer)
                ModularLoadBalancer->GroupLeader = GroupLeader;

        UpdateCache();
        ForceNetUpdate();
    }
}

TArray<UFGFactoryConnectionComponent*> ALBBuild_ModularLoadBalancer::GetConnections(EFactoryConnectionDirection Direction) const
{
    if(!GroupLeader)
        return {};
    
    TArray<UFGFactoryConnectionComponent*> Return;
    TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> Array = Direction == EFactoryConnectionDirection::FCD_INPUT ? GroupLeader->mConnectedInputs : GroupLeader->mConnectedOutputs;;
    
    for (TWeakObjectPtr<ALBBuild_ModularLoadBalancer> LoadBalancer : Array)
        if(LoadBalancer.IsValid())
            Return.Add(Direction == EFactoryConnectionDirection::FCD_INPUT ? LoadBalancer->MyInputConnection : LoadBalancer->MyOutputConnection);
    
    return Return;
}

TArray<ALBBuild_ModularLoadBalancer*> ALBBuild_ModularLoadBalancer::GetGroupModules() const
{
    if(!GroupLeader)
        return {};
    
    TArray<ALBBuild_ModularLoadBalancer*> Return;
    TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> Array = GroupLeader->mGroupModules;

    if(Array.Num() > 0)
        for (TWeakObjectPtr<ALBBuild_ModularLoadBalancer> ModularLoadBalancer : Array)
            if(ModularLoadBalancer.IsValid())
                Return.Add(ModularLoadBalancer.Get());
    return Return;
}

void ALBBuild_ModularLoadBalancer::Factory_Tick(float dt)
{
    Super::Factory_Tick(dt);

    if(IsLeader() && HasAuthority())
    {
        // not 100% sure if this needed (just going sure here)
        mTimer += dt;
        if(mTimer >= 5.f)
            UpdateCache();

        if(mConnectedOutputs.Num() > 0 && mOutputInventory)
        {
            if(!mConnectedOutputs.IsValidIndex(mOutputIndex))
                mOutputIndex = 0;

            // if still BREAK! something goes wrong here!
            if(!mConnectedOutputs.IsValidIndex(mOutputIndex))
            {
                UE_LOG(LoadBalancers_Log, Error, TEXT("mConnectedOutputs has still a invalid Index!"));
                return;
            }
            if(!mConnectedOutputs[mOutputIndex].IsValid())
                return;

            if(!mOutputInventory || mOutputInventory->IsEmpty())
                return;
        
            if(!SendItemsToOutputs(dt, mConnectedOutputs[mOutputIndex].Get()))
            {
                if(CanSendToOverflow())
                {
                    int Index = mOutputInventory->GetFirstIndexWithItem();
                    FInventoryStack Stack;
                    mOutputInventory->GetStackFromIndex(Index, Stack);
                    GetOverflowLoader()->GetBufferInventory()->AddItem(Stack.Item);
                    mOutputInventory->RemoveFromIndex(Index, 1);
                }
            }
            
            mOutputIndex++;
        }
    }
}

bool ALBBuild_ModularLoadBalancer::CanSendToOverflow() const
{
    bool CanSend = HasOverflowLoader();
    if(CanSend)
    {
        const int Index = mOutputInventory->GetFirstIndexWithItem();
        if(Index >= 0)
        {
            FInventoryStack Stack;
            mOutputInventory->GetStackFromIndex(Index, Stack);
            CanSend = Stack.HasItems() ? GetOverflowLoader()->GetBufferInventory()->HasEnoughSpaceForItem(Stack.Item) : false;

            if(CanSend)
                for (ALBBuild_ModularLoadBalancer* GroupModule : GetGroupModules())
                {
                    if(GroupModule->GetBufferInventory()->HasEnoughSpaceForItem(Stack.Item) && Stack.HasItems())
                    {
                        CanSend = false;
                        break;
                    }
                }
        }
    }
    return CanSend;
}

bool ALBBuild_ModularLoadBalancer::SendItemsToOutputs(float dt, ALBBuild_ModularLoadBalancer* Balancer)
{
    if(Balancer->GetBufferInventory() && mOutputInventory)
    {
        const int Index = mOutputInventory->GetFirstIndexWithItem();
        if(Index >= 0)
        {
            FInventoryStack Stack;
            mOutputInventory->GetStackFromIndex(Index, Stack);
            if(Stack.HasItems() && Balancer->GetBufferInventory()->HasEnoughSpaceForItem(Stack.Item))
            {
                Balancer->GetBufferInventory()->AddItem(Stack.Item);
                mOutputInventory->RemoveFromIndex(Index,1 );
                return true;
            }
        }
    }
    return false;
}

void ALBBuild_ModularLoadBalancer::UpdateCache()
{
    mTimer = .0f;

    TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> ConnectedOutputs = {};
    TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> ConnectedInputs = {};
    
    for (ALBBuild_ModularLoadBalancer* GroupModule : GetGroupModules())
    {
        if(GroupModule->MyInputConnection)
            if(GroupModule->MyInputConnection->IsConnected())
                ConnectedInputs.AddUnique(GroupModule);
                
        if(GroupModule->MyOutputConnection)
            if(GroupModule->MyOutputConnection->IsConnected())
                ConnectedOutputs.AddUnique(GroupModule);
    }

    mConnectedOutputs = ConnectedOutputs;
    mConnectedInputs = ConnectedInputs;
}

void ALBBuild_ModularLoadBalancer::Destroyed()
{
    if (IsLeader())
        if (GetGroupModules().Num() > 1)
            for (ALBBuild_ModularLoadBalancer* LoadBalancer : GetGroupModules())
                if(LoadBalancer != this)
                {
                    LoadBalancer->ApplyLeader();
                    break;
                }

    ForceNetUpdate();
    Super::Destroyed();
}

void ALBBuild_ModularLoadBalancer::ApplyGroupModule()
{
    if(HasAuthority() && GroupLeader)
    {
        GroupLeader->mGroupModules.AddUnique(this);
        if(IsOverflowLoader())
        {
            GroupLeader->mOverflowLoader = this;
        }

        UpdateCache();
        
        ForceNetUpdate();
    }
    else
        UE_LOG(LoadBalancers_Log, Error, TEXT("Cannot Apply GroupLeader: Invalid"))
}

void ALBBuild_ModularLoadBalancer::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    for (UActorComponent* ComponentsByClass : GetComponentsByClass(UFGFactoryConnectionComponent::StaticClass()))
        if(UFGFactoryConnectionComponent* ConnectionComponent = Cast<UFGFactoryConnectionComponent>(ComponentsByClass))
        {
            if(ConnectionComponent->GetDirection() == EFactoryConnectionDirection::FCD_INPUT)
            {
                MyInputConnection = ConnectionComponent;
            }
            else if(ConnectionComponent->GetDirection() == EFactoryConnectionDirection::FCD_OUTPUT)
            {
                MyOutputConnection = ConnectionComponent;
            }
        }
}

void ALBBuild_ModularLoadBalancer::Factory_CollectInput_Implementation()
{
    Super::Factory_CollectInput_Implementation();
    
    if(!IsLeader() || !HasAuthority())
        return;

    if(mConnectedInputs.Num() == 0)
        return;

    if(!mConnectedInputs.IsValidIndex(mInputIndex))
        mInputIndex = 0;

    // if still BREAK! something goes wrong here!
    if(!mConnectedInputs.IsValidIndex(mInputIndex))
    {
        UE_LOG(LoadBalancers_Log, Error, TEXT("mConnectedInputs has still a invalid Index!"));
        return;
    }

    CollectInput(mConnectedInputs[mInputIndex].Get());
    mInputIndex++;
}

void ALBBuild_ModularLoadBalancer::CollectInput(ALBBuild_ModularLoadBalancer* Module)
{
    UFGFactoryConnectionComponent* connection = Module->MyInputConnection;
    if(!connection || !GroupLeader)
        return;

    if(connection->IsConnected() && GroupLeader->mOutputInventory)
    {
        TArray<FInventoryItem> peeker;
        if (connection->Factory_PeekOutput(peeker))
        {
            if(connection->GetInventory() != GroupLeader->mOutputInventory)
                connection->SetInventory(GroupLeader->mOutputInventory);
            
            FInventoryItem Item = peeker[0];
            float offset;
            if(GroupLeader->mOutputInventory->HasEnoughSpaceForItem(Item))
                if(connection->Factory_GrabOutput(Item, offset))
                    GroupLeader->mOutputInventory->AddItem(Item);
        }
    }
}
