#pragma once

#include "ScrollMode.generated.h"


UENUM(BlueprintType)
enum class EScrollMode : uint8
{
	RotateX,
	RotateY,
	RotateZ,
	Scale,
};
ENUM_RANGE_BY_COUNT(EScrollMode, (int) EScrollMode::Scale + 1)

class ScrollMode
{
public:

	static EScrollMode Get() { return activeMode; }
	static void Set(EScrollMode mode) { activeMode = mode; }
	static bool IsRotate() { return activeMode == EScrollMode::RotateX || activeMode == EScrollMode::RotateY || activeMode == EScrollMode::RotateZ; }
	static bool IsScale() { return activeMode == EScrollMode::Scale; }
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

	static const FText GetName() { return GetName(activeMode); }
	static const FText GetName(EScrollMode mode)
	{
		switch (mode)
		{
		case EScrollMode::RotateX: return FText::FromString("Rotate Pitch");
		case EScrollMode::RotateY: return FText::FromString("Rotate Roll");
		case EScrollMode::RotateZ: return FText::FromString("Rotate Yaw");
		case EScrollMode::Scale:   return FText::FromString("Scale");
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
		case EScrollMode::Scale:
		default:				   return EScrollMode::RotateZ;
		}
	}
	inline static EScrollMode activeMode = EScrollMode::RotateZ;
};