#pragma once
#include "Async/AsyncWork.h"
#include "UnifiedGridAsync.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UnifiedGridAsyncLibrary.generated.h"

class UnifiedGridAsyncTask : public FNonAbandonableTask {
public:
	UnifiedGridAsyncTask(UObject* object) { this->object = object; }
	UObject* object;
	FORCEINLINE TStatId GetStatId() const {
		RETURN_QUICK_DECLARE_CYCLE_STAT(UnifiedGridAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}
	void DoWork()
	{
		IUnifiedGridAsync::Execute_UnifiedGridAsyncFunction(object);
	}
};
/**
 * 
 */
UCLASS()
class UNIFIEDGRID_API UUnifiedGridAsyncLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
		static void CallUnifiedGridAsyncFunction(UObject* object) {
		(new FAutoDeleteAsyncTask<UnifiedGridAsyncTask>(object))->StartBackgroundTask();
	}
};
