// Copyright Epic Games, Inc. All Rights Reserved.

#include "ToUwu.h"
#include "Internationalization/Regex.h"
#include "FGBuildDescriptor.h"
#include "Buildables/FGBuildable.h"

#define LOCTEXT_NAMESPACE "FToUwuModule"
DEFINE_LOG_CATEGORY(ToUwu_Log);

FString RegexReplace(
    const FString& Input,
    const FString& Pattern,
    const FString& Replacement)
{
    FString Result;

    FRegexPattern RegexPattern(Pattern);
    FRegexMatcher Matcher(RegexPattern, Input);

    int32 LastIndex = 0;

    while (Matcher.FindNext())
    {
        Result += Input.Mid(
            LastIndex,
            Matcher.GetMatchBeginning() - LastIndex);

        FString ExpandedReplacement = Replacement;

        for (int32 GroupIndex = 1; GroupIndex < 10; ++GroupIndex)
        {
            FString Capture = Matcher.GetCaptureGroup(GroupIndex);

            if (Capture.IsEmpty())
            {
                continue;
            }

            ExpandedReplacement.ReplaceInline(
                *FString::Printf(TEXT("$%d"), GroupIndex),
                *Capture);
        }

        Result += ExpandedReplacement;

        LastIndex = Matcher.GetMatchEnding();
    }

    Result += Input.Mid(LastIndex);

    return Result;
}

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
            //UE_LOG(ToUwu_Log, Display, TEXT("Key: %s. Value: %s"), *key, oldString);
            auto newString = UwuifyString(*oldString);
            PatchStringTableEntry(st.TableId, key, newString);
        }
    }

    PatchDescriptors();
    PatchBuildables();
}

void FToUwuModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FToUwuModule::PatchStringTableEntry(FName TableId, FString Key, FString NewValue)
{
	FStringTableConstPtr TablePtr = FStringTableRegistry::Get()
		.FindStringTable(TableId);

	if (!TablePtr.IsValid()) return;

	FStringTable* MutableTable = const_cast<FStringTable*>(TablePtr.Get());
	if (!MutableTable) return;

	MutableTable->SetSourceString(Key, NewValue);
}

void FToUwuModule::PatchDescriptors()
{
    TArray<UClass*> Results;
    TArray<UObject*> OutCDOs;

    GetDerivedClasses(UFGBuildDescriptor::StaticClass(), Results, true);

    for (UClass* Class : Results)
    {
        if (!Class->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
        {
            OutCDOs.Add(Class->GetDefaultObject());
        }
    }

    for (auto cdo : OutCDOs)
    {
        auto desc = Cast<UFGBuildDescriptor>(cdo);
        if (desc)
        {
            UE_LOG(ToUwu_Log, Display, TEXT("mDisplayName: %s"), *desc->mDisplayName.ToString());
            UE_LOG(ToUwu_Log, Display, TEXT("mDescription: %s"), *desc->mDescription.ToString());
            desc->mDisplayName = FText::FromString(UwuifyString(desc->mDisplayName.ToString()));
            desc->mDescription = FText::FromString(UwuifyString(desc->mDescription.ToString()));
        }
    }
}

void FToUwuModule::PatchBuildables()
{
    TArray<UClass*> Results;
    TArray<UObject*> OutCDOs;

    GetDerivedClasses(AFGBuildable::StaticClass(), Results, true);

    for (UClass* Class : Results)
    {
        if (!Class->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
        {
            OutCDOs.Add(Class->GetDefaultObject());
        }
    }

    for (auto cdo : OutCDOs)
    {
        auto desc = Cast<AFGBuildable>(cdo);
        if (desc)
        {
            UE_LOG(ToUwu_Log, Display, TEXT("mDisplayName: %s"), *desc->mDisplayName.ToString());
            UE_LOG(ToUwu_Log, Display, TEXT("mDescription: %s"), *desc->mDescription.ToString());
            desc->mDisplayName = FText::FromString(UwuifyString(desc->mDisplayName.ToString()));
            desc->mDescription = FText::FromString(UwuifyString(desc->mDescription.ToString()));
        }
    }
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



FString FToUwuModule::UwuifyString(const FString& Input)
{
    FString Result = Input;

    TArray<FString> PreservedTags;

    {
        FRegexPattern TagPattern(TEXT("(<[^>]*>|\\{[^\\}]*\\})"));
        FRegexMatcher Matcher(TagPattern, Result);

        int32 TagIndex = 0;

        while (Matcher.FindNext())
        {
            FString Tag = Matcher.GetCaptureGroup(0);

            FString Placeholder =
                FString::Printf(TEXT("%%UWUTAG%d%%"), TagIndex);

            PreservedTags.Add(Tag);

            Result.ReplaceInline(*Tag, *Placeholder);

            ++TagIndex;
        }
    }

    for (TCHAR& Char : Result)
    {
        switch (Char)
        {
        case TEXT('r'):
        case TEXT('l'):
            Char = TEXT('w');
            break;

        case TEXT('R'):
        case TEXT('L'):
            Char = TEXT('W');
            break;
        }
    }
    //Result.ReplaceInline(TEXT("r"), TEXT("w"));
    //Result.ReplaceInline(TEXT("l"), TEXT("w"));
    //Result.ReplaceInline(TEXT("R"), TEXT("W"));
    //Result.ReplaceInline(TEXT("L"), TEXT("W"));
    Result.ReplaceInline(TEXT("qui"), TEXT("kwi"), ESearchCase::CaseSensitive);
    Result.ReplaceInline(TEXT("Qui"), TEXT("Kwi"), ESearchCase::CaseSensitive);

    Result = RegexReplace(Result, TEXT("n([aeiou])"), TEXT("ny$1"));
    Result = RegexReplace(Result, TEXT("N([aeiouAEIOU])"), TEXT("Ny$1"));

    Result = RegexReplace(Result, TEXT("ove"), TEXT("uv"));
    Result = RegexReplace(Result, TEXT("OVE"), TEXT("UV"));

    Result = RegexReplace(Result, TEXT("th"), TEXT("d"));
    Result = RegexReplace(Result, TEXT("Th"), TEXT("D"));

    Result = RegexReplace(Result, TEXT("!"), TEXT("! owo"));
    Result = RegexReplace(Result, TEXT("\\?"), TEXT("? uwu"));
    //Result = RegexReplace(Result, TEXT("\\."), TEXT(". uwu"));

    for (int32 i = 0; i < PreservedTags.Num(); ++i)
    {
        FString Placeholder =
            FString::Printf(TEXT("%%UWUTAG%d%%"), i);

        Result.ReplaceInline(
            *Placeholder,
            *PreservedTags[i]);
    }

    return Result;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FToUwuModule, ToUwu)