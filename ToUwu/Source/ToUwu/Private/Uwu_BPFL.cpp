#include "Uwu_BPFL.h"

FString UUwu_BPFL::RegexReplace(const FString& Input, const FString& Pattern, const FString& Replacement)
{
    FString Result;

    FRegexPattern RegexPattern(Pattern);
    FRegexMatcher Matcher(RegexPattern, Input);

    int32 LastIndex = 0;

    while (Matcher.FindNext())
    {
        Result += Input.Mid(LastIndex, Matcher.GetMatchBeginning() - LastIndex);

        FString ExpandedReplacement = Replacement;

        for (int32 GroupIndex = 1; GroupIndex < 10; ++GroupIndex)
        {
            FString Capture = Matcher.GetCaptureGroup(GroupIndex);

            if (Capture.IsEmpty())
            {
                continue;
            }

            ExpandedReplacement.ReplaceInline(*FString::Printf(TEXT("$%d"), GroupIndex), *Capture);
        }

        Result += ExpandedReplacement;

        LastIndex = Matcher.GetMatchEnding();
    }

    Result += Input.Mid(LastIndex);

    return Result;
}

void UUwu_BPFL::PatchDescriptors()
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
            desc->mDisplayName = FText::FromString(UwuifyString(desc->mDisplayName.ToString()));
            desc->mDescription = FText::FromString(UwuifyString(desc->mDescription.ToString()));
        }
    }
}

void UUwu_BPFL::PatchBuildables()
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
            desc->mDisplayName = FText::FromString(UwuifyString(desc->mDisplayName.ToString()));
            desc->mDescription = FText::FromString(UwuifyString(desc->mDescription.ToString()));
        }
    }
}

FString UUwu_BPFL::UwuifyString(const FString & Input)
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

            FString Placeholder = FString::Printf(TEXT("%%UWUTAG%d%%"), TagIndex);

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

    //Result.ReplaceInline(TEXT("r"), TEXT("w"), ESearchCase::CaseSensitive);
    //Result.ReplaceInline(TEXT("l"), TEXT("w"), ESearchCase::CaseSensitive);
    //Result.ReplaceInline(TEXT("R"), TEXT("W"), ESearchCase::CaseSensitive);
    //Result.ReplaceInline(TEXT("L"), TEXT("W"), ESearchCase::CaseSensitive);

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
        FString Placeholder = FString::Printf(TEXT("%%UWUTAG%d%%"), i);

        Result.ReplaceInline(*Placeholder, *PreservedTags[i]);
    }

    return Result;
}
