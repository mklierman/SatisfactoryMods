#pragma once

#include "ScrollMode.generated.h"


UENUM(BlueprintType)
enum class EScrollMode : uint8
{
	RotateX,
	RotateY,
	RotateZ,
	Scale,
	ScaleX,
	ScaleY,
	ScaleZ,
};
ENUM_RANGE_BY_COUNT(EScrollMode, (int) EScrollMode::ScaleZ + 1)

class ScrollMode
{
public:

	static EScrollMode Get() { return activeMode; }
	static void Set(EScrollMode mode) { activeMode = mode; }
	static bool IsRotate() { return activeMode == EScrollMode::RotateX || activeMode == EScrollMode::RotateY || activeMode == EScrollMode::RotateZ; }
	static bool IsScale() { return activeMode == EScrollMode::Scale || activeMode == EScrollMode::ScaleX || activeMode == EScrollMode::ScaleY || activeMode == EScrollMode::ScaleZ; }
	static void Reset() { activeMode = EScrollMode::RotateZ; }
	static void Cycle() { activeMode = Next(activeMode); }

	static EAxis::Type GetRotationAxis()
	{
		switch (activeMode)
		{
		case EScrollMode::RotateX: return EAxis::X;
		case EScrollMode::RotateY: return EAxis::Y;
		case EScrollMode::RotateZ: return EAxis::Z;
		default:				   return EAxis::None;
		}
	}

	static EAxis::Type GetScaleAxis()
	{
		switch (activeMode)
		{
		case EScrollMode::ScaleX: return EAxis::X;
		case EScrollMode::ScaleY: return EAxis::Y;
		case EScrollMode::ScaleZ: return EAxis::Z;
		default:                  return EAxis::None;
		}
	}

	static const FText GetName() { return GetName(activeMode); }
	static const FText GetName(EScrollMode mode)
	{
		switch (mode)
		{
		case EScrollMode::RotateX: return FText::FromString("Rotate Pitch");
		case EScrollMode::RotateY: return FText::FromString("Rotate Roll");
		case EScrollMode::RotateZ: return FText::FromString("Rotate Yaw");
		case EScrollMode::Scale:   return FText::FromString("Scale");
		case EScrollMode::ScaleX:  return FText::FromString("Scale X");
		case EScrollMode::ScaleY:  return FText::FromString("Scale Y");
		case EScrollMode::ScaleZ:  return FText::FromString("Scale Z");
		default:				   return FText::FromString("Unknown");
		}
	}

private:
	static EScrollMode Next(EScrollMode current)
	{
		switch (current)
		{
		case EScrollMode::RotateX: return EScrollMode::Scale;
		case EScrollMode::RotateY: return EScrollMode::RotateX;
		case EScrollMode::RotateZ: return EScrollMode::RotateY;
		case EScrollMode::Scale:   return EScrollMode::ScaleX;
		case EScrollMode::ScaleX:  return EScrollMode::ScaleY;
		case EScrollMode::ScaleY:  return EScrollMode::ScaleZ;
		case EScrollMode::ScaleZ:
		default:				   return EScrollMode::RotateZ;
		}
	}
	inline static EScrollMode activeMode = EScrollMode::RotateZ;
};
