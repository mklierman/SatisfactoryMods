#include "LBBPLib.h"

void ULBBPLib::SortItemArray(TArray<TSubclassOf<UFGItemDescriptor>>& Out_Items, const TArray<TSubclassOf<UFGItemDescriptor>>& In_Items, const TArray<TSubclassOf<UFGItemDescriptor>>& mForceFirstItems, bool Reverse)
{
	Out_Items = In_Items;
	if(Out_Items.Num() > 1)
	{
		Out_Items.Sort([Reverse](const TSubclassOf<UFGItemDescriptor> A, const TSubclassOf<UFGItemDescriptor> B)
		{
			if(!Reverse)
				return UFGItemDescriptor::GetItemName(A).ToString() < UFGItemDescriptor::GetItemName(B).ToString();
			return UFGItemDescriptor::GetItemName(A).ToString() > UFGItemDescriptor::GetItemName(B).ToString();
		});

		if(mForceFirstItems.Num() > 0)
		{
			TArray<TSubclassOf<UFGItemDescriptor>> ForceReturn;
			for (TSubclassOf<UFGItemDescriptor> Out_Item : mForceFirstItems)
			{
				if(Out_Items.Contains(Out_Item))
				{
					Out_Items.Remove(Out_Item);
					ForceReturn.Add(Out_Item);
				}
			}
			ForceReturn.Append(Out_Items);
			Out_Items = ForceReturn;
		}
	}
}
