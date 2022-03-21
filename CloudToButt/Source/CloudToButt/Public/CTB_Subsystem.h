

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include <Runtime/UMG/Public/Components/Widget.h>
#include "CTB_Subsystem.generated.h"

/**
 *
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWidgetConstruct, UUserWidget*, Widget);

UCLASS()
class CLOUDTOBUTT_API ACTB_Subsystem : public AModSubsystem
{
	GENERATED_BODY()
public:
		UPROPERTY(BlueprintAssignable)
		FOnWidgetConstruct OnWidgetConstruct;
};
