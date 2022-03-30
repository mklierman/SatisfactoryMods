


#include "LB_BPFL.h"

void ULB_BPFL::SortStringArray(UPARAM(ref)TArray<FString>& Array_To_Sort, TArray<FString>& Sorted_Array)
{
	Array_To_Sort.Sort();

	Sorted_Array = Array_To_Sort;
}