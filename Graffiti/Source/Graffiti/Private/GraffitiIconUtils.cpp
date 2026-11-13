#include "GraffitiIconUtils.h"

#include "Engine/Texture.h"
#include "FGIconDatabaseSubsystem.h"
#include "FGIconLibrary.h"

UTexture* Graffiti::ResolveIconTexture(UObject* WorldContext, const FPersistentGlobalIconId& IconId)
{
	AFGIconDatabaseSubsystem* Database = AFGIconDatabaseSubsystem::Get(WorldContext);
	if (!Database || !Database->IsInitialized())
	{
		return nullptr;
	}

	const int32 LocalIconId = Database->ResolvePersistentGlobalIconId(IconId);
	FIconData IconData;
	if (LocalIconId == INDEX_NONE || !Database->GetIconDataForIconID(LocalIconId, IconData))
	{
		return nullptr;
	}

	return Cast<UTexture>(UFGIconLibrary::GetTextureFromIconData(IconData));
}
