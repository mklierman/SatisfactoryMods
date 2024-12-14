#pragma once

#include "FGFactoryColoringTypes.h"
#include "PP_ActorComponent.generated.h"

UCLASS(Blueprintable)
class PERSISTENTPAINTABLES_API UPP_ActorComponent : public UActorComponent
{
	GENERATED_BODY()
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(BlueprintReadWrite, Replicated)
	FFactoryCustomizationData CustomizationStruct;
};

