// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Internationalization/StringTable.h"
#include "Internationalization/StringTableRegistry.h"
#include "StringTableRegistry.h"
#include "StringTableCore.h"
#include "Uwu_BPFL.h"

DECLARE_LOG_CATEGORY_EXTERN(ToUwu_Log, Display, All);

struct FStringTableDump
{
	FName TableId;
	TMap<FString, FString> Entries;
};

class FToUwuModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TArray<FStringTableDump> DumpAllStringTables();

	void PatchStringTableEntry(FName TableId, FString Key, FString NewValue);
};
