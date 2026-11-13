#include "GraffitiGameInstanceSubsystem.h"

#include "Engine/GameInstance.h"
#include "GraffitiRemoteCallObject.h"
#include "Registry/RemoteCallObjectRegistry.h"

void UGraffitiGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<URemoteCallObjectRegistry>();

	if (URemoteCallObjectRegistry* Registry = GetGameInstance()->GetSubsystem<URemoteCallObjectRegistry>())
	{
		Registry->RegisterRemoteCallObject(UGraffitiRemoteCallObject::StaticClass());
	}
}
