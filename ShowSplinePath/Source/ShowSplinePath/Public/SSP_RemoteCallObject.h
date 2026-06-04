#pragma once

#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "SSP_RemoteCallObject.generated.h"

UCLASS(NotBlueprintable)
class SHOWSPLINEPATH_API USSP_RemoteCallObject : public UFGRemoteCallObject
{
    GENERATED_BODY()

public:
    USSP_RemoteCallObject();


private:
    UPROPERTY(Replicated)
    int32 DummyReplicatedField;
};