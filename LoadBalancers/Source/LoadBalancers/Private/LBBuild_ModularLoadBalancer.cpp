


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

    ApplyGroupModule(this);
}

void ALBBuild_ModularLoadBalancer::ApplyLeader(ALBBuild_ModularLoadBalancer* OldLeader)
{
    if(HasAuthority())
    {
        if(OldLeader->GetBufferInventory() && GetBufferInventory())
        {
            GetBufferInventory()->CopyFromOtherComponent(OldLeader->GetBufferInventory());
            GetBufferInventory()->Resize(FMath::Max<int>((GetGroupModules().Num() - 1) * 2, 2));
        }

        GroupLeader = this;

        for (ALBBuild_ModularLoadBalancer* ModularLoadBalancer : GetGroupModules())
            if(ModularLoadBalancer)
                ModularLoadBalancer->GroupLeader = GroupLeader;

        UpdateCache();
    }
}

TArray<UFGFactoryConnectionComponent*> ALBBuild_ModularLoadBalancer::GetConnections(EFactoryConnectionDirection Direction) const
{
    TArray<UFGFactoryConnectionComponent*> Return;

    TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> Array;
    if(IsLeader())
        Array = Direction == EFactoryConnectionDirection::FCD_INPUT ? mConnectedInputs : mConnectedOutputs;
    else if(GroupLeader)
        Array = Direction == EFactoryConnectionDirection::FCD_INPUT ? GroupLeader->mConnectedInputs : GroupLeader->mConnectedOutputs;;
    
    for (TWeakObjectPtr<ALBBuild_ModularLoadBalancer> LoadBalancer : Array)
        if(LoadBalancer.IsValid())
            Return.Add(Direction == EFactoryConnectionDirection::FCD_INPUT ? LoadBalancer->MyInputConnection : LoadBalancer->MyOutputConnection);
    
    return Return;
}

TArray<ALBBuild_ModularLoadBalancer*> ALBBuild_ModularLoadBalancer::GetGroupModules() const
{
    TArray<ALBBuild_ModularLoadBalancer*> Return;

    TArray<TWeakObjectPtr<ALBBuild_ModularLoadBalancer>> Array;
    if(IsLeader())
        Array = mGroupModules;
    else if(GroupLeader)
        Array = GroupLeader->mGroupModules;
    
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
        mTimer += dt;
        if(mTimer >= 5.f)
            UpdateCache();
        
        mOutputIndex++;
        if(!mConnectedOutputs.IsValidIndex(mOutputIndex))
            mOutputIndex = 0;

        // if still BREAK! something goes wrong here!
        if(!mConnectedOutputs.IsValidIndex(mOutputIndex))
        {
            UE_LOG(LoadBalancers_Log, Error, TEXT("mConnectedInputs has still a invalid Index!"));
            return;
        }

        bool HasPushed = false;
        TArray<ALBBuild_ModularLoadBalancer*> CanPush = {};
        for(int idx = mOutputIndex; idx < mConnectedOutputs.Num(); ++idx)
        {
            if(mConnectedOutputs[idx].IsValid())
            {
                ALBBuild_ModularLoadBalancer* Balancer = mConnectedOutputs[idx].Get();
                if(CanPushOutput(Balancer))
                {
                    if(idx >= mOutputIndex)
                    {
                        PushOutput(Balancer);
                        HasPushed = true;
                    }
                    else
                        CanPush.AddUnique(Balancer);
                }
            }
        }

        if(CanPush.Num() > 0 && !HasPushed)
            if(CanPush[0])
                PushOutput(CanPush[0]);
    }
}


void ALBBuild_ModularLoadBalancer::PushOutput(ALBBuild_ModularLoadBalancer* Module)
{
    FInventoryStack Stack;
    GetBufferInventory()->GetStackFromIndex(GetBufferInventory()->GetFirstIndexWithItem(), Stack);
    if(Stack.HasItems())
    {
        float Offset = .0f;
        if(Module->MyOutputConnection->Factory_GrabOutput(Stack.Item, Offset))
            GetBufferInventory()->RemoveFromIndex(GetBufferInventory()->GetFirstIndexWithItem(), 1);
    }
}

bool ALBBuild_ModularLoadBalancer::CanPushOutput(ALBBuild_ModularLoadBalancer* Module) const
{
    if(GroupLeader)
        if(Module && GroupLeader->GetBufferInventory())
            if(Module->MyOutputConnection && !GroupLeader->GetBufferInventory()->IsEmpty())
                if(Module->MyOutputConnection->IsConnected())
                {
                    TArray< FInventoryItem > Out;
                    return !Module->MyOutputConnection->Factory_PeekOutput(Out);
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
                    LoadBalancer->ApplyLeader(this);
                    break;
                }

    ForceNetUpdate();
    Super::Destroyed();
}

void ALBBuild_ModularLoadBalancer::ApplyGroupModule(ALBBuild_ModularLoadBalancer* Balancer)
{
    if(HasAuthority() && GroupLeader)
    {
        if(Balancer)
        {
            GroupLeader->mGroupModules.AddUnique(Balancer);

            if(GroupLeader->GetBufferInventory())
                GroupLeader->GetBufferInventory()->Resize(FMath::Max<int>(GetGroupModules().Num() * 2, 2));

            UpdateCache();
        }
        else
            UE_LOG(LoadBalancers_Log, Error, TEXT("Cannot Apply module: Invalid"))
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
    if(!IsLeader() || !HasAuthority())
        return;

    mInputIndex++;
    if(!mConnectedInputs.IsValidIndex(mInputIndex))
        mInputIndex = 0;

    // if still BREAK! something goes wrong here!
    if(!mConnectedInputs.IsValidIndex(mInputIndex))
    {
        UE_LOG(LoadBalancers_Log, Error, TEXT("mConnectedInputs has still a invalid Index!"));
        return;
    }

    if(mConnectedInputs[mInputIndex].IsValid())
        CollectInput(mConnectedInputs[mInputIndex].Get());
}

void ALBBuild_ModularLoadBalancer::CollectInput(ALBBuild_ModularLoadBalancer* Module)
{
    UFGFactoryConnectionComponent* connection = Module->MyInputConnection;
    if(!connection || !GroupLeader)
        return;

    if(connection->IsConnected() && GroupLeader->GetBufferInventory())
    {
        TArray<FInventoryItem> peeker;
        if (connection->Factory_PeekOutput(peeker))
        {
            FInventoryItem Item = peeker[0];
            if(GroupLeader->GetBufferInventory()->HasEnoughSpaceForItem(Item))
                if(connection->Factory_Internal_GrabOutputInventory(Item, UFGItemDescriptor::StaticClass()))
                    GroupLeader->GetBufferInventory()->AddItem(Item);
        }
    }
}

bool ALBBuild_ModularLoadBalancer::Factory_GrabOutput_Implementation(UFGFactoryConnectionComponent* connection,
    FInventoryItem& out_item, float& out_OffsetBeyond, TSubclassOf<UFGItemDescriptor> type)
{
    return false;
}
