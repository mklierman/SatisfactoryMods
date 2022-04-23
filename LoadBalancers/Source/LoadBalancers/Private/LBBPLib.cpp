#include "LBBPLib.h"

void ULBBPLib::SortItemArray(TArray<TSubclassOf<UFGItemDescriptor>>& Out_Items, const TArray<TSubclassOf<UFGItemDescriptor>>& In_Items, bool Reverse)
{
	if(In_Items.Num() > 1)
	{
		TMap<FString, TSubclassOf<UFGItemDescriptor>> Map;

		FCriticalSection Mutex;
		ParallelFor(In_Items.Num(), [&](int32 Idx)
		{
			if(In_Items.IsValidIndex(Idx))
			{
				Mutex.Lock();
				TSubclassOf<UFGItemDescriptor> Item = In_Items[Idx];
				Map.Add(UFGItemDescriptor::GetItemName(Item).ToString(), Item);
				Mutex.Unlock();
			}
		});

		TArray<FString> Keys;
		Map.GenerateKeyArray(Keys);
		Keys.Sort([Reverse](const FString A, const FString B)
		{
			if(!Reverse)
				return A < B;
			return A > B;
		});

		for (FString Name : Keys)
			Out_Items.Add(Map[Name]);
	}
	else
		Out_Items = In_Items;
}
