


#include "LBBuild_ModularLoadBalancer.h"
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
    DOREPLIFETIME(ALBBuild_ModularLoadBalancer, GroupModules);
    DOREPLIFETIME(ALBBuild_ModularLoadBalancer, GroupLeader);
    DOREPLIFETIME(ALBBuild_ModularLoadBalancer, SnappedBalancer);
    DOREPLIFETIME(ALBBuild_ModularLoadBalancer, DefaultLoadBalancerMaterials);
    DOREPLIFETIME(ALBBuild_ModularLoadBalancer, TempLoadBalancerMaterial);
    DOREPLIFETIME(ALBBuild_ModularLoadBalancer, Mesh);
}

void ALBBuild_ModularLoadBalancer::BeginPlay()
{
    Super::BeginPlay();
    this->SetActorTickEnabled(true);
    if (this->GroupLeader)
    {
        this->GroupLeader->InputMap.Add(MyInputConnection, 0);
        this->GroupLeader->InputConnections.AddUnique(MyInputConnection);
        this->GroupLeader->OutputMap.Add(MyOutputConnection, 0);
        this->GroupLeader->GroupModules.AddUnique(this);

        //UE_LOG(LoadBalancers_Log, Display, TEXT("Leader Group Modules: %f"), (float)this->GroupLeader->GroupModules.Num());
        //UE_LOG(LoadBalancers_Log, Display, TEXT("Group Leader InputMaps: %f"), (float)this->GroupLeader->InputMap.Num());
        //UE_LOG(LoadBalancers_Log, Display, TEXT("Group Leader OutputMaps: %f"), (float)this->GroupLeader->OutputMap.Num());
        //UE_LOG(LoadBalancers_Log, Display, TEXT("Group Leader InputConnections: %f"), (float)this->GroupLeader->InputConnections.Num());
        //UE_LOG(LoadBalancers_Log, Display, TEXT("Group Leader Buffer Size: %f"), (float)bufferSize);
    }

    ForceNetUpdate();
}

void ALBBuild_ModularLoadBalancer::Destroyed()
{
    if (this->GroupLeader == this)
    {
        if (this->InputConnections.Num() > 1)
        {
            ALBBuild_ModularLoadBalancer* newLeader = Cast<ALBBuild_ModularLoadBalancer>(this->InputConnections[1]->GetOuterBuildable());
            if (newLeader)
            {
                newLeader->GroupLeader = newLeader;

                //Figure out how to refund

                newLeader->Buffer->Resize(this->Buffer->GetSizeLinear() - 2);

                newLeader->InputConnections = this->InputConnections;
                newLeader->OutputMap = this->OutputMap;
                newLeader->InputMap = this->InputMap;
                newLeader->GroupModules = this->GroupModules;

                newLeader->InputConnections.Remove(this->InputConnections[0]);
                newLeader->OutputMap.Remove(this->MyOutputConnection);
                newLeader->InputMap.Remove(this->MyInputConnection);
                newLeader->GroupModules.Remove(this);

                for (auto conn : newLeader->InputConnections)
                {
                    //newLeader->InputQueue.Enqueue(conn);
                    ALBBuild_ModularLoadBalancer* balancer = Cast<ALBBuild_ModularLoadBalancer>(conn->GetOuterBuildable());
                    if (balancer)
                    {
                        balancer->GroupLeader = newLeader;
                    }
                }
            }
        }
    }
    else
    {
        if (this->GroupLeader && this->GroupLeader->Buffer->GetSizeLinear() > 4)
        {
            this->GroupLeader->Buffer->Resize(this->GroupLeader->Buffer->GetSizeLinear() - 2);
        }

        for (auto conn : this->InputConnections)
        {
            if (this->GroupLeader && this->GroupLeader->InputMap.Contains(conn))
            {
                this->GroupLeader->InputMap.Remove(conn);
            }

            if (this->GroupLeader && this->GroupLeader->InputConnections.Contains(conn))
            {
                this->GroupLeader->InputConnections.Remove(conn);
            }
        }

        TArray<UFGFactoryConnectionComponent*> conns;
        this->OutputMap.GetKeys(conns);
        for (auto conn : conns)
        {
            if (this->GroupLeader && this->GroupLeader->OutputMap.Contains(conn))
            {
                this->GroupLeader->OutputMap.Remove(conn);
            }
        }

        TArray<UFGFactoryConnectionComponent*> inputconns;
        this->InputMap.GetKeys(inputconns);
        for (auto conn : inputconns)
        {
            if (this->GroupLeader && this->GroupLeader->InputMap.Contains(conn))
            {
                this->GroupLeader->InputMap.Remove(conn);
            }
        }

        if (this->GroupLeader && this->GroupLeader->GroupModules.Contains(this))
        {
            this->GroupLeader->GroupModules.Remove(this);
        }
    }
    ForceNetUpdate();
    Super::Destroyed();
}

void ALBBuild_ModularLoadBalancer::Tick(float dt)
{
    Super::Tick(dt);

    //Check if a belt has been disconnected and reset the map value if so
    if (MyOutputConnection && !MyOutputConnection->IsConnected())
    {
        if (this->GroupLeader)
        {
            if (this->GroupLeader->OutputMap.Contains(MyOutputConnection))
            {
                this->GroupLeader->OutputMap[MyOutputConnection] = 0;
            }
        }
    }

    TArray<FInventoryItem> peeker;
    if (MyInputConnection)
    {
        if (!MyInputConnection->IsConnected())
        {
            if (this->GroupLeader)
            {
                if (this->GroupLeader->InputMap.Contains(MyInputConnection))
                {
                    this->GroupLeader->InputMap[MyInputConnection] = 0;
                }
            }
        }
        else if (MyInputConnection->IsConnected() && MyInputConnection->mConnectedComponent && MyInputConnection->mConnectedComponent->GetInventory() && MyInputConnection->mConnectedComponent->GetInventory()->IsEmpty())
        {
            if (this->GroupLeader)
            {
                if (this->GroupLeader->InputMap.Contains(MyInputConnection))
                {
                    this->GroupLeader->InputMap[MyInputConnection] = 0;
                }
            }
        }
    }
}

bool ALBBuild_ModularLoadBalancer::ShouldSave_Implementation() const
{
    return true;
}

void ALBBuild_ModularLoadBalancer::SetCustomizationData_Native(const FFactoryCustomizationData& customizationData)
{
    Super::SetCustomizationData_Native(customizationData);
    //Code for when customization data gets propogated. Currently issues with new balancers being added to group
    //if (this->GroupLeader == this)
    //{
    //    TArray< UFGFactoryConnectionComponent*> groupConns;
    //    this->OutputMap.GenerateKeyArray(groupConns);
    //    for (auto conn : groupConns)
    //    {
    //        ALBBuild_ModularLoadBalancer* bal = Cast<ALBBuild_ModularLoadBalancer>(conn->GetOuterBuildable());
    //        if (bal)
    //        {
    //            bal->LocalSetCustomizationData_Native(customizationData);
    //        }
    //    }
    //}
    //else
    //{
    //    this->GroupLeader->SetCustomizationData_Native(customizationData);
    //}
}
void ALBBuild_ModularLoadBalancer::ApplyCustomizationData_Native(const FFactoryCustomizationData& customizationData)
{
    Super::ApplyCustomizationData_Native(customizationData);
    //Code for when customization data gets propogated. Currently issues with new balancers being added to group
    /*if (this->GroupLeader == this)
    {
        TArray< UFGFactoryConnectionComponent*> groupConns;
        this->OutputMap.GenerateKeyArray(groupConns);
        for (auto conn : groupConns)
        {
            ALBBuild_ModularLoadBalancer* bal = Cast<ALBBuild_ModularLoadBalancer>(conn->GetOuterBuildable());
            if (bal)
            {
                bal->LocalApplyCustomizationData_Native(customizationData);
            }
        }
    }
    else
    {
        this->GroupLeader->ApplyCustomizationData_Native(customizationData);
    }*/
}

void ALBBuild_ModularLoadBalancer::LocalSetCustomizationData_Native(const FFactoryCustomizationData& customizationData)
{
    Super::SetCustomizationData_Native(customizationData);
}

void ALBBuild_ModularLoadBalancer::LocalApplyCustomizationData_Native(const FFactoryCustomizationData& customizationData)
{
    Super::ApplyCustomizationData_Native(customizationData);
}

void ALBBuild_ModularLoadBalancer::Factory_CollectInput_Implementation()
{
    if (this->GroupLeader == this)
    {
        this->CollectInput(MyInputConnection);
    }
    else
    {
        GroupLeader->CollectInput(MyInputConnection);
    }
}

void ALBBuild_ModularLoadBalancer::CollectInput(UFGFactoryConnectionComponent* connection)
{
    bool result = false;
    TArray<FInventoryItem> peeker;
    if (this->GroupLeader == this)
    {
        if (connection->IsConnected() && connection->Factory_PeekOutput(peeker))
        {
            TArray<int> ValueArray;
            InputMap.GenerateValueArray(ValueArray);
            int MaxInt;
            int MaxIntIndex;
            if (InputMap.Find(connection))
            {
                FScopeLock ScopeLock(&mInputLock);
                int InputCalls = *InputMap.Find(connection);
                int emptyBufferIndex = Buffer->FindEmptyIndex();
                if (emptyBufferIndex >= 0)
                {
                    UKismetMathLibrary::MaxOfIntArray(ValueArray, MaxIntIndex, MaxInt);
                    if (InputCalls == MaxInt)
                    {
                        FInventoryItem OutItem;
                        bool OutBool;
                        float out_OffsetBeyond = 100.f;
                        OutBool = connection->Factory_GrabOutput(OutItem, out_OffsetBeyond);
                        if (OutBool)
                        {
                            FInventoryStack Stack = UFGInventoryLibrary::MakeInventoryStack(1, OutItem);
                            Buffer->AddStackToIndex(emptyBufferIndex, Stack, false);
                            InputMap[connection] = 0;
                        }

                    }
                    else
                    {
                        InputMap[connection] = InputCalls + 1;
                    }
                }
                else
                {
                    InputMap[connection] = InputCalls + 1;
                }
            }
        }
        else
        {
            if (OutputMap.Find(connection))
            {
                OutputMap[connection] = 0;
            }
        }
    }
}

void ALBBuild_ModularLoadBalancer::GiveOutput(bool& result, FInventoryItem& out_item)
{
    if (this->GroupLeader == this)
    {
        auto mBufferIndex = Buffer->GetFirstIndexWithItem();
        if (mBufferIndex >= 0)
        {
            FInventoryStack Stack;
            result = Buffer->GetStackFromIndex(mBufferIndex, Stack);
            if (result && Stack.HasItems())
            {
                Buffer->RemoveAllFromIndex(mBufferIndex);
                int numItems;
                UFGInventoryLibrary::BreakInventoryStack(Stack, numItems, out_item);
            }
        }
    }
}

bool ALBBuild_ModularLoadBalancer::Factory_GrabOutput_Implementation(UFGFactoryConnectionComponent* connection, FInventoryItem& out_item, float& out_OffsetBeyond, TSubclassOf<UFGItemDescriptor> type)
{
    bool result = false;
    if (this->GroupLeader == this)
    {
        if (connection->IsConnected())
        {
            TArray<int> ValueArray;
            OutputMap.GenerateValueArray(ValueArray);
            int MaxInt;
            int MaxIntIndex;
            if (OutputMap.Find(connection))
            {
                int OutputCalls = *OutputMap.Find(connection);
                if (!Buffer->IsEmpty())
                {
                    FScopeLock ScopeLock(&mOutputLock);
                    UKismetMathLibrary::MaxOfIntArray(ValueArray, MaxIntIndex, MaxInt);
                    if (OutputCalls == MaxInt)
                    {
                        GiveOutput(result, out_item);
                        OutputMap[connection] = 0;
                    }
                    else
                    {
                        OutputMap[connection] = OutputCalls + 1;
                    }
                }
                else
                {
                    OutputMap[connection] = OutputCalls + 1;
                }
            }
        }
        else
        {
            if (OutputMap.Find(connection))
            {
                OutputMap[connection] = 0;
            }
        }
        return result;
    }
    else
    {
        if (this->GroupLeader)
        {
            return this->GroupLeader->Factory_GrabOutput_Implementation(connection, out_item, out_OffsetBeyond, type);
        }
    }
    return result;
}
