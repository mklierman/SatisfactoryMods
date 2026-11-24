#pragma once

#include "CoreMinimal.h"

namespace Graffiti
{
	inline constexpr float MinSize = 25.0f;
	inline constexpr float MaxSize = 5000.0f;
	inline constexpr float MinDistance = 100.0f;
	inline constexpr float MaxDistance = 3000.0f;
	inline constexpr float MinGridSize = 1.0f;
	inline constexpr float MaxGridSize = 1000.0f;
	// Allow decals to reach recessed sections of layered building meshes.
	inline constexpr float DecalDepth = 100.0f;
	inline constexpr float SurfaceOffset = 1.0f;
	inline constexpr float PreviewFadeScreenSize = 0.002f;
	inline constexpr float LayerNormalAlignment = 0.9f;
	inline constexpr float LayerPlaneTolerance = 25.0f;
	inline constexpr float SquareRadiusScale = 0.70710678f;
	inline constexpr int32 PreviewSortOrder = MAX_int32;

	inline constexpr TCHAR DecalMaterialPath[] = TEXT("/Graffiti/Materials/M_GraffitiDecal.M_GraffitiDecal");
	inline const FName IconTextureParameter = TEXT("IconTexture");
	inline const FName MirrorParameter = TEXT("MirrorHorizontal");
}
