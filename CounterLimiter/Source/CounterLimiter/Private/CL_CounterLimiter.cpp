
#include "CL_CounterLimiter.h"
#include "FGFactoryConnectionComponent.h"
#include "FGInventoryLibrary.h"

DEFINE_LOG_CATEGORY(CounterLimiter_Log);

void ACL_CounterLimiter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACL_CounterLimiter, inputConnection);
	DOREPLIFETIME(ACL_CounterLimiter, outputConnection);
	DOREPLIFETIME(ACL_CounterLimiter, OutputStageBuffer);
	DOREPLIFETIME(ACL_CounterLimiter, DisplayIPM);
	DOREPLIFETIME(ACL_CounterLimiter, InputBuffer);
}

ACL_CounterLimiter::ACL_CounterLimiter()
{
	//this->GetBufferInventory()->Resize(1);
}


void ACL_CounterLimiter::BeginPlay()
{
	Super::BeginPlay();
	//mGameSecondsAtLastInput = this->GetGameTimeSinceCreation();
	this->mInputs.AddUnique(inputConnection);
	this->mOutputs.AddUnique(outputConnection);
	//this->GetBufferInventory()->Resize(1);
	//this->mBufferInventory->Resize(1);
	GetWorld()->GetTimerManager().SetTimer(ipmTimerHandle, this, &ACL_CounterLimiter::CalculateIPM, 60.f, true, 60.f);
	SetThroughputLimit(mPerMinuteLimitRate);
}

bool ACL_CounterLimiter::ShouldSave_Implementation() const
{
	return true;
}

void ACL_CounterLimiter::Factory_CollectInput_Implementation()
{
	int emptyBufferIndex = InputBuffer->FindEmptyIndex();
	if (emptyBufferIndex >= 0)
	{
		FInventoryItem OutItem;
		bool OutBool;
		float out_OffsetBeyond = 100.f;
		OutBool = inputConnection->Factory_GrabOutput(OutItem, out_OffsetBeyond);
		if (OutBool)
		{
			FInventoryStack Stack = UFGInventoryLibrary::MakeInventoryStack(1, OutItem);
			InputBuffer->AddStackToIndex(emptyBufferIndex, Stack, false);
		}
	}
}

bool ACL_CounterLimiter::Factory_GrabOutput_Implementation(UFGFactoryConnectionComponent* connection, FInventoryItem& out_item, float& out_OffsetBeyond, TSubclassOf<UFGItemDescriptor> type)
{
	if (mPerMinuteLimitRate < 0)
	{
		StageItemForOutput();
	}
	//FScopeLock ScopeLock(&mOutputLock);
	bool result = false;
	auto mBufferIndex = OutputStageBuffer->GetFirstIndexWithItem();
	if (mBufferIndex >= 0)
	{
		FInventoryStack Stack;
		result = OutputStageBuffer->GetStackFromIndex(mBufferIndex, Stack);
		if (result && Stack.HasItems())
		{
			//UE_LOG(CounterLimiter_Log, Display, TEXT("GetSecondsSinceLastInput() > GetSecondsPerItem() : %f > %f"), GetSecondsSinceLastInput(), GetSecondsPerItem());
			//mGameSecondsAtLastInput = this->GetGameTimeSinceCreation();
			OutputStageBuffer->RemoveAllFromIndex(mBufferIndex);
			int numItems;
			UFGInventoryLibrary::BreakInventoryStack(Stack, numItems, out_item);
			ItemCount++;
		}
	}
	return result;
}

float ACL_CounterLimiter::GetSecondsPerItem()
{
	return 1.0f/(mPerMinuteLimitRate/60.0f);
}

float ACL_CounterLimiter::GetSecondsSinceLastInput()
{
	return this->GetGameTimeSinceCreation() - mGameSecondsAtLastInput;
}

int32 ACL_CounterLimiter::GetCurrentPerMinuteThroughput()
{
	return mCurrentPerMinuteThroughput;
}

void ACL_CounterLimiter::SetThroughputLimit(float itemsPerMinute)
{
	mPerMinuteLimitRate = itemsPerMinute;

	GetWorld()->GetTimerManager().SetTimer(ThroughputTimerHandle, this, &ACL_CounterLimiter::StageItemForOutput, GetSecondsPerItem(), true);
}

void ACL_CounterLimiter::CalculateIPM()
{
	//UE_LOG(CounterLimiter_Log, Display, TEXT("CalculateIPM - ItemCount: %f"), (float)ItemCount);
	if (ItemCount > 0)
	{
		DisplayIPM = ((float)ItemCount / 60.f) * 60.f;
		ItemCount = 0;
	}
	else
	{
		DisplayIPM = 0.f;
	}
}

void ACL_CounterLimiter::StageItemForOutput()
{
	int emptyBufferIndex = OutputStageBuffer->FindEmptyIndex();
	if (emptyBufferIndex >= 0)
	{
		auto mBufferIndex = InputBuffer->GetFirstIndexWithItem();
		if (mBufferIndex >= 0)
		{
			FInventoryStack Stack;
			bool result = InputBuffer->GetStackFromIndex(mBufferIndex, Stack);
			if (result && Stack.HasItems() )
			{
				//mGameSecondsAtLastInput = this->GetGameTimeSinceCreation();
				InputBuffer->RemoveAllFromIndex(mBufferIndex);
				int numItems;
				FInventoryItem OutItem;
				UFGInventoryLibrary::BreakInventoryStack(Stack, numItems, OutItem);

				float out_OffsetBeyond = 100.f;
				//FInventoryStack Stack = UFGInventoryLibrary::MakeInventoryStack(1, OutItem);
				OutputStageBuffer->AddStackToIndex(emptyBufferIndex, Stack, false);
			}
		}
	}
}
