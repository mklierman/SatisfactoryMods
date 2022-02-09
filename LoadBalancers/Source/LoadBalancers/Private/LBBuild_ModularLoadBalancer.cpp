


#include "LBBuild_ModularLoadBalancer.h"
#include "FGFactoryConnectionComponent.h"
#include "FGInventoryLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "LoadBalancersModule.h"

DEFINE_LOG_CATEGORY(LoadBalancers_Log);
ALBBuild_ModularLoadBalancer::ALBBuild_ModularLoadBalancer()
{
}

void ALBBuild_ModularLoadBalancer::BeginPlay()
{
	Super::BeginPlay();
    this->mInventorySizeX = 2;
    this->mInventorySizeY = 2;
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

                newLeader->InputConnections.Remove(this->InputConnections[0]);
                newLeader->OutputMap.Remove(this->MyOutputConnection);

                for (auto conn : newLeader->InputConnections)
                {
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
    }
    Super::Destroyed();
}

void ALBBuild_ModularLoadBalancer::Tick(float dt)
{
    Super::Tick(dt);


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
}

bool ALBBuild_ModularLoadBalancer::ShouldSave_Implementation() const
{
    return true;
}

void ALBBuild_ModularLoadBalancer::Factory_CollectInput_Implementation()
{
    if (this->GroupLeader == this)
    {
        for (int i = 0; i < InputConnections.Num(); i++)
        {
            UFGFactoryConnectionComponent* Input = Cast<UFGFactoryConnectionComponent>(InputConnections[i]);
            if (Input->IsConnected())
            {
                int emptyBufferIndex = Buffer->FindEmptyIndex();
                if (emptyBufferIndex >= 0)
                {
                    FInventoryItem OutItem;
                    bool OutBool;
                    float out_OffsetBeyond;
                    OutBool = Input->Factory_GrabOutput(OutItem, out_OffsetBeyond);
                    if (OutBool)
                    {
                        FInventoryStack Stack = UFGInventoryLibrary::MakeInventoryStack(1, OutItem);
                        Buffer->AddStackToIndex(emptyBufferIndex, Stack, false);
                    }
                }
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
                    FScopeLock ScopeLock(&mLock);
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
