#pragma once
#include "Module/GameInstanceModule.h"
#include "InfiniteNudgeGameInstanceModule.generated.h"

UCLASS(Blueprintable)
class INFINITENUDGE_API UInfiniteNudgeGameInstanceModule : public UGameInstanceModule {
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, Category = "Advanced | Input")
    TMap<UInputAction*, FGameplayTag> InputActionTagBindings;
};