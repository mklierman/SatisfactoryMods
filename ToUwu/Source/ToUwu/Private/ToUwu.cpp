// Copyright Epic Games, Inc. All Rights Reserved.

#include "ToUwu.h"
#include "Internationalization/Regex.h"
//#include "FGBuildDescriptor.h"
#include "Buildables/FGBuildable.h"
#include "Internationalization/StringTableCore.h"

#define LOCTEXT_NAMESPACE "FToUwuModule"
DEFINE_LOG_CATEGORY(ToUwu_Log);

void FToUwuModule::StartupModule()
{
    TArray<FStringTableDump> stringTables = DumpAllStringTables();

    for (FStringTableDump st : stringTables)
    {
        auto tableid = st.TableId.ToString();
        //UE_LOG(ToUwu_Log, Display, TEXT("String table id: %s"), *tableid);
        TArray<FString> keys;
        st.Entries.GenerateKeyArray(keys);
        for (FString key : keys)
        {
            auto oldString = st.Entries.Find(key);
            auto newString = UUwu_BPFL::UwuifyString(*oldString);
            PatchStringTableEntry(st.TableId, key, newString);
        }
    }
}

void FToUwuModule::ShutdownModule()
{
}

void FToUwuModule::PatchStringTableEntry(FName TableId, FString Key, FString NewValue)
{
	FStringTableConstPtr TablePtr = FStringTableRegistry::Get().FindStringTable(TableId);

    if (!TablePtr.IsValid())
    {
        return;
    }

	FStringTable* MutableTable = const_cast<FStringTable*>(TablePtr.Get());
    if (!MutableTable)
    {
        return;
    }

	MutableTable->SetSourceString(Key, NewValue);
}

TArray<FStringTableDump> FToUwuModule::DumpAllStringTables()
{
    TArray<FStringTableDump> Result;

    FStringTableRegistry::Get().EnumerateStringTables(
        [&Result](const FName& TableId, const FStringTableConstRef& StringTable) -> bool
        {
            FStringTableDump& Dump = Result.AddDefaulted_GetRef();
            Dump.TableId = TableId;

            StringTable->EnumerateSourceStrings(
                [&Dump](const FString& Key, const FString& SourceString) -> bool
                {
                    Dump.Entries.Add(Key, SourceString);
                    return true;
                }
            );

            return true;
        }
    );

    return Result;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FToUwuModule, ToUwu)