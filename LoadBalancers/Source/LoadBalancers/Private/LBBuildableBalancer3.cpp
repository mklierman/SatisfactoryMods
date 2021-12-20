
#include "LBBuildableBalancer3.h"
#include "FGFactoryConnectionComponent.h"
#include "FGInventoryLibrary.h"
#include "Kismet/KismetMathLibrary.h"
//#include "Kismet/KismetAnimationLibrary.h"
#include "CoreMinimal.h"

DEFINE_LOG_CATEGORY(LoadBalancers_Log);

ALBBuildableBalancer3::ALBBuildableBalancer3()
{
}

void ALBBuildableBalancer3::BeginPlay()
{
    //Buffer->SetDefaultSize(4);
    Super::BeginPlay();
}

void ALBBuildableBalancer3::Factory_Tick(float deltaTime)
{
    Super::Factory_Tick(deltaTime);
}

void ALBBuildableBalancer3::Factory_CollectInput_Implementation()
{
   // auto startTime = FDateTime::Now();
    int emptyBufferIndex = Buffer->FindEmptyIndex();
    if (emptyBufferIndex >= 0)
    {
        for (int i = 0; i < InputConnections.Num(); i++)
        {
            UFGFactoryConnectionComponent* Input = Cast<UFGFactoryConnectionComponent>(InputConnections[i]);
            if (Input->IsConnected())
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
   // auto endTime = FDateTime::Now();
   // auto length = endTime - startTime;
   // UE_LOG(LoadBalancers_Log, Display, TEXT("Factory_CollectInput_Implementation took %f microseconds"), length.GetTotalMicroseconds());
}

void ALBBuildableBalancer3::GiveOutput(bool& result, FInventoryItem& out_item)
{
    mBufferIndex = Buffer->GetFirstIndexWithItem();
    //UE_LOG(LoadBalancers_Log, Display, TEXT("Buffer Not Empty"));
    FInventoryStack Stack;
    result = Buffer->GetStackFromIndex(mBufferIndex, Stack);
    Buffer->RemoveAllFromIndex(mBufferIndex);
    int numItems;
    UFGInventoryLibrary::BreakInventoryStack(Stack, numItems, out_item);
}

bool ALBBuildableBalancer3::Factory_GrabOutput_Implementation(UFGFactoryConnectionComponent* connection, FInventoryItem& out_item, float& out_OffsetBeyond, TSubclassOf<UFGItemDescriptor> type)
{
    //UE_LOG(LoadBalancers_Log, Display, TEXT("Factory_GrabOutput_Implementation"));
    //auto startTime = FDateTime::Now();
    TArray<int> ValueArray;
    OutputMap.GenerateValueArray(ValueArray);
    int MaxInt;
    int ThrowAway;
    UKismetMathLibrary::MaxOfIntArray(ValueArray, ThrowAway, MaxInt);
    bool result = false;
    int OutputCalls = *OutputMap.Find(connection);
    if (!Buffer->IsEmpty())
    {
        if (OutputCalls == MaxInt)
        {
            OutputMap.Add(connection, 0);
            GiveOutput(result, out_item);
            //OutputQueue.Pop();
        }
        else
        {
            OutputMap.Add(connection, OutputCalls+1);
        }
    }
    else
    {
        OutputMap.Add(connection, OutputCalls+1);
    }
    //auto endTime = FDateTime::Now();
    //auto length = endTime - startTime;
    //UE_LOG(LoadBalancers_Log, Display, TEXT("Factory_GrabOutput_Implementation took %f microseconds"), length.GetTotalMicroseconds());
    return result;
}

bool ALBBuildableBalancer3::HandleGrabOutput(UFGFactoryConnectionComponent* OutputConnection, FInventoryItem& out_item, float& out_OffsetBeyond)
{
    //UE_LOG(LoadBalancers_Log, Display, TEXT("HandleGrabOutput"));
    bool result = false;
    if (Buffer->IsEmpty())
    {
        //UE_LOG(LoadBalancers_Log, Display, TEXT("Buffer 1 Empty"));
        mBufferIndex = 0;
        for (int i = 0; i < InputConnections.Num() - 1; i++)
        {
            //UE_LOG(LoadBalancers_Log, Display, TEXT("In For Loop"));
            UFGFactoryConnectionComponent* Input = Cast<UFGFactoryConnectionComponent>(InputConnections[i]);
            if (Input && Input->IsConnected())
            {
                //UE_LOG(LoadBalancers_Log, Display, TEXT("Input Connected"));
                FInventoryItem OutItem;
                bool OutBool;
                OutBool = Input->Factory_GrabOutput(OutItem, out_OffsetBeyond);
                if (OutBool)
                {
                    //UE_LOG(LoadBalancers_Log, Display, TEXT("Grabbed Item"));
                    mBufferIndex++;
                    FInventoryStack Stack = UFGInventoryLibrary::MakeInventoryStack(1, OutItem);
                    Buffer->AddStackToIndex(mBufferIndex, Stack, false);
                }
            }
        }
    }

    if (!Buffer->IsEmpty())
    {
        //UE_LOG(LoadBalancers_Log, Display, TEXT("Buffer 2 Not Empty"));
        FInventoryStack Stack;
        result = Buffer->GetStackFromIndex(mBufferIndex, Stack);
        Buffer->RemoveAllFromIndex(mBufferIndex);
        if (mBufferIndex >= 1)
        {
            //UE_LOG(LoadBalancers_Log, Display, TEXT("Reducing Buffer Index"));
            mBufferIndex--;
        }
        //FInventoryItem OutItem;
        int numItems;
        UFGInventoryLibrary::BreakInventoryStack(Stack, numItems, out_item);
    }
    //UE_LOG(LoadBalancers_Log, Display, TEXT("Returning"));
    return result;
}
