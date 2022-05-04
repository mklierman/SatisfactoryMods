
#include "CL_CounterLimiter.h"
#include "FGFactoryConnectionComponent.h"
#include "FGInventoryLibrary.h"
#include "FGIconLibrary.h"

//DEFINE_LOG_CATEGORY(CounterLimiter_Log);

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

	if (HasAuthority())
	{
		this->mInputs.AddUnique(inputConnection);
		this->mOutputs.AddUnique(outputConnection);
		//this->GetBufferInventory()->Resize(1);
		//this->mBufferInventory->Resize(1);
		GetWorld()->GetTimerManager().SetTimer(ipmTimerHandle, this, &ACL_CounterLimiter::CalculateIPM, 60.f, true, 60.f);
		SetThroughputLimit(mPerMinuteLimitRate);
	}
}

void ACL_CounterLimiter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		for (UActorComponent* ComponentsByClass : GetComponentsByClass(UFGInventoryComponent::StaticClass()))
		{
			if (UFGInventoryComponent* InventoryComponent = Cast<UFGInventoryComponent>(ComponentsByClass))
			{
				InventoryComponent->Resize(1);
			}
		}
	}
}

bool ACL_CounterLimiter::ShouldSave_Implementation() const
{
	return true;
}

void ACL_CounterLimiter::Factory_CollectInput_Implementation()
{
	if (HasAuthority())
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
}

bool ACL_CounterLimiter::Factory_GrabOutput_Implementation(UFGFactoryConnectionComponent* connection, FInventoryItem& out_item, float& out_OffsetBeyond, TSubclassOf<UFGItemDescriptor> type)
{
	if (HasAuthority())
	{
		if (mPerMinuteLimitRate < 0)
		{
			StageItemForOutput();
		}

		bool result = false;
		auto mBufferIndex = OutputStageBuffer->GetFirstIndexWithItem();
		if (mBufferIndex >= 0)
		{
			FInventoryStack Stack;
			result = OutputStageBuffer->GetStackFromIndex(mBufferIndex, Stack);
			if (result && Stack.HasItems())
			{
				OutputStageBuffer->RemoveAllFromIndex(mBufferIndex);
				int numItems;
				UFGInventoryLibrary::BreakInventoryStack(Stack, numItems, out_item);
				ItemCount++;

				if (mItemCounts.Contains(out_item.ItemClass))
				{
					int32 newValue = *mItemCounts.Find(out_item.ItemClass) + 1;
					mItemCounts[out_item.ItemClass] = newValue;
				}
				else
				{
					mItemCounts.Add(out_item.ItemClass, 1);
				}
			}
		}
		return result;
	}
	return false;
}

float ACL_CounterLimiter::GetSecondsPerItem()
{
	return 1.0f/(mPerMinuteLimitRate/60.0f);
}

float ACL_CounterLimiter::GetSecondsSinceLastInput()
{
	return this->GetGameTimeSinceCreation() - mGameSecondsAtLastInput;
}

void ACL_CounterLimiter::UpdateAttachedSigns()
{
	if (HasAuthority() && mAttachedSigns.Num() > 0)
	{
		//TMap<FText, int32> iconNames;
		auto iconLib = UFGIconLibrary::Get();
		auto icons = iconLib->GetIconData();
		//for (FIconData icon : icons)
		//{
		//	//iconNames.Add(icon.IconName, icon.ID);
		//}

		for (auto sign : mAttachedSigns)
		{
			if (sign)
			{
				FPrefabSignData out_signData;
				sign->GetSignPrefabData(out_signData);
				TMap<FString, FString> textMap = out_signData.TextElementData;
				TArray<FString> keys;
				textMap.GetKeys(keys);
				for (int i = 0; i < keys.Num(); i++)
				{
					textMap[keys[i]] = FString::FromInt((int)DisplayIPM);
				}
				out_signData.TextElementData = textMap;

				if (mItemCounts.Num() > 0)
				{
					TSubclassOf<class UFGItemDescriptor> itemDesc;
					int32 highestCount = 0;

					for (auto& item : mItemCounts)
					{
						if (item.Value > highestCount)
						{
							itemDesc = item.Key;
							highestCount = item.Value;
						}
					}
					auto itemName = UFGItemDescriptor::GetItemName(itemDesc);
					int32 iconInt = -1;
					for (auto icon : icons)
					{
						if (icon.IconName.ToString() == itemName.ToString())
						{
							iconInt = icon.ID;
						}
					}
					if (iconInt >= 0)
					{
						TMap<FString, int32> iconMap = out_signData.IconElementData;
						iconMap["Icon"] = iconInt;
						out_signData.IconElementData = iconMap;
					}
				}
				sign->SetPrefabSignData(out_signData);

				//Key: Icon / Value: 194 (iron ingot)
			}
			else
			{
				mAttachedSigns.Remove(sign);
			}
		}
	}
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
	OnUpdateDisplay.Broadcast(DisplayIPM);
	UpdateAttachedSigns();
	mItemCounts.Reset();
}

void ACL_CounterLimiter::StageItemForOutput()
{
	if (HasAuthority())
	{
		int emptyBufferIndex = OutputStageBuffer->FindEmptyIndex();
		if (emptyBufferIndex >= 0)
		{
			auto mBufferIndex = InputBuffer->GetFirstIndexWithItem();
			if (mBufferIndex >= 0)
			{
				FInventoryStack Stack;
				bool result = InputBuffer->GetStackFromIndex(mBufferIndex, Stack);
				if (result && Stack.HasItems())
				{
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
}
