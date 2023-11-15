
#include "CL_CounterLimiter.h"
#include "FGFactoryConnectionComponent.h"
#include "FGInventoryLibrary.h"
#include "CL_RCO.h"
#include "FGIconLibrary.h"
#include "Buildables/FGBuildableConveyorBelt.h"
#include <Net/UnrealNetwork.h>

//DEFINE_LOG_CATEGORY(CounterLimiter_Log);

//#pragma optimize("", off)
void ACL_CounterLimiter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACL_CounterLimiter, DisplayIPM);
	DOREPLIFETIME(ACL_CounterLimiter, mPerMinuteLimitRate);
	DOREPLIFETIME(ACL_CounterLimiter, OnUpdateDisplay);
}

ACL_CounterLimiter::ACL_CounterLimiter()
{
}


void ACL_CounterLimiter::BeginPlay()
{

	if (HasAuthority())
	{
		//auto conns = this->GetConnectionComponents();
		//for (auto con : conns)
		//{
		//	if (con->GetDirection() == EFactoryConnectionDirection::FCD_INPUT)
		//	{

		//	}
		//}
		//this->mInputs.AddUnique(inputConnection);
		//this->mOutputs.AddUnique(outputConnection);
		//this->mInventorySizeX = 1;
		//this->mInventorySizeY = 1;
		InputBuffer->SetLocked(false);
		InputBuffer->Resize(1);
		inputConnection->SetInventory(this->GetBufferInventory());
		inputConnection->GetInventory()->SetLocked(false);
		OutputStageBuffer->SetLocked(false);
		outputConnection->SetInventory(OutputStageBuffer);
		inputConnection->SetInventoryAccessIndex(0);
		outputConnection->SetInventoryAccessIndex(0);
		GetWorld()->GetTimerManager().SetTimer(ipmTimerHandle, this, &ACL_CounterLimiter::CalculateIPM, 60.f, true, 60.f);
		SetThroughputLimit(mPerMinuteLimitRate, true);
	}
	Super::BeginPlay();
}

void ACL_CounterLimiter::Dismantle_Implementation()
{
	if (inputConnection && outputConnection)
	{
		if (inputConnection->GetConnection() && outputConnection->GetConnection())
		{
			auto inputLoc = inputConnection->GetConnection()->GetConnectorLocation();
			auto outputLoc = outputConnection->GetConnection()->GetConnectorLocation();
			if ((inputLoc - outputLoc).IsNearlyZero(0.01))
			{
				auto belt1Conn = inputConnection->GetConnection();
				auto belt2Conn = outputConnection->GetConnection();
				if (belt1Conn && belt2Conn)
				{
					auto belt1 = Cast< AFGBuildableConveyorBelt>(belt1Conn->GetOuterBuildable());
					auto belt2 = Cast< AFGBuildableConveyorBelt>(belt2Conn->GetOuterBuildable());
					if (belt1 && belt2)
					{
						if (belt1->GetSpeed() == belt2->GetSpeed())
						{
							auto belt1CustomizationData = belt1->Execute_GetCustomizationData(belt1);
							TArray< AFGBuildableConveyorBelt*> belts;
							belts.Add(belt1);
							belts.Add(belt2);
							belt1Conn->ClearConnection();
							belt2Conn->ClearConnection();
							belt1Conn->SetConnection(belt2Conn);
							auto newBelt = AFGBuildableConveyorBelt::Merge(belts);
							newBelt->Execute_SetCustomizationData(newBelt, belt1CustomizationData);
						}
						else
						{
							belt1Conn->ClearConnection();
							belt2Conn->ClearConnection();
							belt1Conn->SetConnection(belt2Conn);
						}
					}
				}
			}
		}
	}
	Super::Dismantle_Implementation();
}

void ACL_CounterLimiter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		//this->mInputs.AddUnique(inputConnection);
		//this->mOutputs.AddUnique(outputConnection);
		InputBuffer->SetLocked(false);
		inputConnection->SetInventory(this->GetBufferInventory());
		OutputStageBuffer->SetLocked(false);
		outputConnection->SetInventory(OutputStageBuffer);
		inputConnection->SetInventoryAccessIndex(0);
		outputConnection->SetInventoryAccessIndex(0);
		//this->mInventorySizeX = 1;
		//this->mInventorySizeY = 1;
		GetWorld()->GetTimerManager().SetTimer(ipmTimerHandle, this, &ACL_CounterLimiter::CalculateIPM, 60.f, true, 60.f);
		SetThroughputLimit(mPerMinuteLimitRate);
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
		inputConnection->GetInventory()->SetLocked(false);
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
		outputConnection->GetInventory()->SetLocked(false);
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

				if (mItemCounts.Contains(out_item.GetItemClass()))
				{
					int32 newValue = *mItemCounts.Find(out_item.GetItemClass()) + 1;
					mItemCounts[out_item.GetItemClass()] = newValue;
				}
				else
				{
					mItemCounts.Add(out_item.GetItemClass(), 1);
				}
			}
		}
		return result;
	}
	return false;
}

float ACL_CounterLimiter::GetSecondsPerItem()
{
	//if (mPerMinuteLimitRate == 0.f)
	//{
	//	return 0.f;
	//}
	return 1.0f / (mPerMinuteLimitRate / 60.0f);
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

UFGInventoryComponent* ACL_CounterLimiter::GetBufferInventory()
{
	return InputBuffer;
}


void ACL_CounterLimiter::SetThroughputLimit(float itemsPerMinute, bool bypassCheck)
{
	if (mPerMinuteLimitRate != itemsPerMinute || bypassCheck)
	{
		if (HasAuthority())
		{
			mPerMinuteLimitRate = itemsPerMinute;
			ForceNetUpdate();
			GetWorld()->GetTimerManager().SetTimer(ThroughputTimerHandle, this, &ACL_CounterLimiter::StageItemForOutput, GetSecondsPerItem(), true);
		}
		else
		{
			mPerMinuteLimitRate = itemsPerMinute;
			if (UCL_RCO* RCO = UCL_RCO::Get(GetWorld()))
			{
				RCO->Server_SetLimiterRate(this, itemsPerMinute);
			}
		}
	}
}

float ACL_CounterLimiter::GetThroughputLimit()
{
	ForceNetUpdate();
	return mPerMinuteLimitRate;
}

float ACL_CounterLimiter::GetCurrentIPM()
{
	return DisplayIPM;
}

void ACL_CounterLimiter::CalculateIPM()
{
	if (HasAuthority())
	{
		ForceNetUpdate();
		UE_LOG(CounterLimiter_Log, Display, TEXT("CalculateIPM - ItemCount: %f"), (float)ItemCount);
		if (ItemCount > 0)
		{
			DisplayIPM = ((float)ItemCount / 60.f) * 60.f;
			ItemCount = 0;
		}
		else
		{
			DisplayIPM = 0.f;
		}
		//UE_LOG(CounterLimiter_Log, Display, TEXT("CalculateIPM - DisplayIPM: %f"), DisplayIPM);
		ForceNetUpdate();
		OnUpdateDisplay.Broadcast(DisplayIPM);
		UpdateAttachedSigns();
		mItemCounts.Reset();
	}
	else
	{
		if (UCL_RCO* RCO = UCL_RCO::Get(GetWorld()))
		{
			//UE_LOG(CounterLimiter_Log, Display, TEXT("Got RCO"));
			RCO->Server_CalculateIPM(this);
		}
	}
}

void ACL_CounterLimiter::OnRep_SetDisplayIPM()
{
	ForceNetUpdate();
	OnUpdateDisplay.Broadcast(DisplayIPM);
}

void ACL_CounterLimiter::StageItemForOutput()
{
	if (HasAuthority())
	{
		int emptyBufferIndex = OutputStageBuffer->FindEmptyIndex();
		if (emptyBufferIndex >= 0)
		{
			if (GetBufferInventory())
			{
				auto mBufferIndex = GetBufferInventory()->GetFirstIndexWithItem();
				if (mBufferIndex >= 0)
				{
					FInventoryStack Stack;
					bool result = GetBufferInventory()->GetStackFromIndex(mBufferIndex, Stack);
					if (result && Stack.HasItems())
					{
						GetBufferInventory()->RemoveAllFromIndex(mBufferIndex);
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
}
void ACL_CounterLimiter::netFunc_SetThroughputLimit(float newLimit)
{
	SetThroughputLimit(newLimit);
}
float ACL_CounterLimiter::netFunc_GetThroughputLimit()
{
	return GetThroughputLimit();
}
float ACL_CounterLimiter::netFunc_GetCurrentIPM()
{
	return GetCurrentIPM();
}
//#pragma optimize("", on)
