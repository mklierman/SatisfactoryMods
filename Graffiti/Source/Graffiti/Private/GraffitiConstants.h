#pragma once

#include "CoreMinimal.h"

namespace Graffiti
{
	inline constexpr float MinSize = 25.0f;
	inline constexpr float MaxSize = 1000.0f;
	inline constexpr float MinDistance = 100.0f;
	inline constexpr float MaxDistance = 3000.0f;
	inline constexpr float MinGridSize = 1.0f;
	inline constexpr float MaxGridSize = 1000.0f;
	inline constexpr float DecalDepth = 8.0f;
	inline constexpr float SurfaceOffset = 1.0f;
	inline constexpr float PreviewFadeScreenSize = 0.002f;
	inline constexpr int32 PreviewSortOrder = 100;

	inline constexpr TCHAR DecalMaterialPath[] = TEXT("/Graffiti/Materials/M_GraffitiDecal.M_GraffitiDecal");
	inline const FName IconTextureParameter = TEXT("IconTexture");
	inline const FName TintParameter = TEXT("Tint");
	inline const FName MirrorParameter = TEXT("MirrorHorizontal");
}
