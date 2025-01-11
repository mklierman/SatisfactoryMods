#pragma once

#include "FGSignTypes.h"
#include "CSS_ActorComponent.generated.h"


UCLASS(Blueprintable)
class UCSS_ActorComponent : public UActorComponent
{
	GENERATED_BODY()
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(BlueprintReadWrite, Replicated)
	FPrefabSignData SignData;
};

