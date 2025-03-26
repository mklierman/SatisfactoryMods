
#pragma once
#include "InterfaceConfiguration.generated.h"

UINTERFACE()
class UArchitectorUI : public UInterface
{
	GENERATED_BODY()
};

class IArchitectorUI
{
	GENERATED_BODY()

public:
	
};

USTRUCT(Blueprintable, BlueprintType)
struct FArchitectorUIObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, SaveGame)
	FVector2D Position = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool AlwaysVisible = true;

	FArchitectorUIObject(){}

	FArchitectorUIObject(FVector2D Position, bool AlwaysVisible)
	{
		this->Position = Position;
		this->AlwaysVisible = AlwaysVisible;
	}
};

USTRUCT(Blueprintable, BlueprintType)
struct FArchitectorKeybindsUI : public FArchitectorUIObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool ShowMovementKeybinds = true;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool ShowRotationKeybinds = true;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool ShowSettingsKeybinds = true;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool ShowOtherKeybinds = true;

	FArchitectorKeybindsUI() : Super(FVector2D(30, 120), true){}
};

USTRUCT(Blueprintable, BlueprintType)
struct FArchitectorStatsUI : public FArchitectorUIObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool ShowPosition = true;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool ShowRotation = true;

	FArchitectorStatsUI() : Super(FVector2D(240, 120), true){}
};

USTRUCT(Blueprintable, BlueprintType)
struct FArchitectorQuickActionsPanel : public FArchitectorUIObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool IsOpen = false;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	int CurrentTabIndex = 0;

	FArchitectorQuickActionsPanel() : Super(FVector2D(500, 120), true){}
};

USTRUCT(Blueprintable, BlueprintType)
struct FArchitectorInterfaceConfiguration
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, SaveGame)
	FArchitectorKeybindsUI KeybindsUI;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	FArchitectorStatsUI StatsUI;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	FArchitectorQuickActionsPanel QuickActionsPanel;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	FArchitectorUIObject ActionHistory = FArchitectorUIObject(FVector2D(1600, 252), true);
};
