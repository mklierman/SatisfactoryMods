#pragma once

class UObject;
class UTexture;
struct FPersistentGlobalIconId;

namespace Graffiti
{
	UTexture* ResolveIconTexture(UObject* WorldContext, const FPersistentGlobalIconId& IconId);
}
