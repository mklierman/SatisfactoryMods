#include "AccessTransformerDocument.h"

#include "AccessTransformerRules.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"

namespace
{
	const TCHAR* SectionName = TEXT("[AccessTransformers]");
}

void FAccessTransformerDocument::Reset()
{
	Entries.Reset();
	Rows.Reset();
	InvalidCount = 0;
	WarningCount = 0;
	CppRebuildCount = 0;
}

bool FAccessTransformerDocument::ParseStructValue(const FString& Value, TMap<FString, FString>& OutFields)
{
	FString Inner = Value;
	Inner.TrimStartAndEndInline();
	if (Inner.StartsWith(TEXT("(")) && Inner.EndsWith(TEXT(")")))
	{
		Inner.MidInline(1, Inner.Len() - 2);
	}
	int32 Index = 0;
	auto SkipSpace = [&Inner, &Index]()
	{
		while (Index < Inner.Len() && FChar::IsWhitespace(Inner[Index]))
		{
			++Index;
		}
	};
	while (Index < Inner.Len())
	{
		SkipSpace();
		const int32 KeyStart = Index;
		while (Index < Inner.Len() && Inner[Index] != TEXT('=') && Inner[Index] != TEXT(','))
		{
			++Index;
		}
		if (Index >= Inner.Len() || Inner[Index] != TEXT('='))
		{
			break;
		}
		FString Key = Inner.Mid(KeyStart, Index - KeyStart);
		Key.TrimStartAndEndInline();
		++Index;
		SkipSpace();
		FString FieldValue;
		if (Index < Inner.Len() && Inner[Index] == TEXT('"'))
		{
			++Index;
			while (Index < Inner.Len() && Inner[Index] != TEXT('"'))
			{
				FieldValue.AppendChar(Inner[Index++]);
			}
			if (Index < Inner.Len())
			{
				++Index;
			}
		}
		else
		{
			const int32 ValueStart = Index;
			while (Index < Inner.Len() && Inner[Index] != TEXT(','))
			{
				++Index;
			}
			FieldValue = Inner.Mid(ValueStart, Index - ValueStart);
			FieldValue.TrimStartAndEndInline();
		}
		if (!Key.IsEmpty())
		{
			OutFields.Add(Key, FieldValue);
		}
		SkipSpace();
		if (Index < Inner.Len() && Inner[Index] == TEXT(','))
		{
			++Index;
		}
	}
	return OutFields.Num() > 0;
}

FString FAccessTransformerDocument::CommentBody(const FString& TrimmedLine)
{
	FString Body = TrimmedLine.RightChop(1);
	if (Body.StartsWith(TEXT(" ")))
	{
		Body.RightChopInline(1);
	}
	return Body;
}

bool FAccessTransformerDocument::LoadFromFile(const FString& Path)
{
	Reset();
	TArray<FString> Lines;
	if (!FFileHelper::LoadFileToStringArray(Lines, *Path))
	{
		return false;
	}
	bool bInSection = true;
	TArray<FString> PendingLines;
	TArray<FString> PendingBodies;
	auto FlushPendingAsRaw = [this, &PendingLines, &PendingBodies]()
	{
		for (const FString& Pending : PendingLines)
		{
			FEntry Entry;
			Entry.RawLine = Pending;
			Entries.Add(Entry);
		}
		PendingLines.Reset();
		PendingBodies.Reset();
	};
	for (const FString& Line : Lines)
	{
		const FString Trimmed = Line.TrimStartAndEnd();
		if (Trimmed.StartsWith(TEXT("[")))
		{
			FlushPendingAsRaw();
			bInSection = Trimmed.Equals(SectionName, ESearchCase::IgnoreCase);
			FEntry Entry;
			Entry.RawLine = Line;
			Entries.Add(Entry);
			continue;
		}
		if (!bInSection || Trimmed.IsEmpty())
		{
			FlushPendingAsRaw();
			FEntry Entry;
			Entry.RawLine = Line;
			Entries.Add(Entry);
			continue;
		}
		if (Trimmed.StartsWith(TEXT(";")))
		{
			PendingLines.Add(Line);
			PendingBodies.Add(CommentBody(Trimmed));
			continue;
		}
		FString Key;
		FString Value;
		if (!Trimmed.Split(TEXT("="), &Key, &Value))
		{
			FlushPendingAsRaw();
			FEntry Entry;
			Entry.RawLine = Line;
			Entries.Add(Entry);
			continue;
		}
		Key.TrimStartAndEndInline();
		Value.TrimStartAndEndInline();
		if (Key.StartsWith(TEXT("+")) || Key.StartsWith(TEXT(".")))
		{
			Key.RightChopInline(1);
			Key.TrimStartAndEndInline();
		}
		EAccessTransformerKind Kind;
		if (!FAccessTransformerRules::KindFromString(Key, Kind))
		{
			FlushPendingAsRaw();
			FEntry Entry;
			Entry.RawLine = Line;
			Entries.Add(Entry);
			continue;
		}
		TMap<FString, FString> Fields;
		ParseStructValue(Value, Fields);
		TSharedPtr<FAccessTransformerEditRow> Row = MakeShared<FAccessTransformerEditRow>();
		Row->Kind = Kind;
		Row->Comment = FString::Join(PendingBodies, TEXT("\n"));
		PendingLines.Reset();
		PendingBodies.Reset();
		if (Kind == EAccessTransformerKind::BlueprintType)
		{
			Row->Target = Fields.FindRef(TEXT("Struct"));
			if (Row->Target.IsEmpty())
			{
				Row->Target = Fields.FindRef(TEXT("Class"));
			}
		}
		else
		{
			Row->Target = Fields.FindRef(TEXT("Class"));
			Row->Member = Fields.FindRef(Kind == EAccessTransformerKind::Friend				 ? TEXT("FriendClass")
										 : Kind == EAccessTransformerKind::BlueprintCallable ? TEXT("Function")
																							 : TEXT("Property"));
		}
		FEntry Entry;
		Entry.bIsRow = true;
		Entry.Row = Row;
		Entries.Add(Entry);
		Rows.Add(Row);
	}
	FlushPendingAsRaw();
	return true;
}

bool FAccessTransformerDocument::SaveToFile(const FString& Path) const
{
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
	return FFileHelper::SaveStringToFile(BuildIniText(), *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}

FString FAccessTransformerDocument::BuildIniText() const
{
	bool bHasSection = false;
	for (const FEntry& Entry : Entries)
	{
		if (!Entry.bIsRow && Entry.RawLine.TrimStartAndEnd().Equals(SectionName, ESearchCase::IgnoreCase))
		{
			bHasSection = true;
			break;
		}
	}
	FString Out;
	if (!bHasSection)
	{
		Out += SectionName;
		Out += LINE_TERMINATOR;
	}
	for (const FEntry& Entry : Entries)
	{
		if (!Entry.bIsRow)
		{
			Out += Entry.RawLine;
			Out += LINE_TERMINATOR;
			continue;
		}
		if (!Entry.Row.IsValid() || Entry.Row->bDuplicate)
		{
			continue;
		}
		if (!Entry.Row->Comment.IsEmpty())
		{
			TArray<FString> CommentLines;
			Entry.Row->Comment.ParseIntoArrayLines(CommentLines);
			for (const FString& CommentLine : CommentLines)
			{
				Out += TEXT("; ") + CommentLine + LINE_TERMINATOR;
			}
		}
		Out += FAccessTransformerRules::FormatIniLine(*Entry.Row) + LINE_TERMINATOR;
	}
	return Out;
}

void FAccessTransformerDocument::AddRow(const TSharedPtr<FAccessTransformerEditRow>& Row)
{
	Rows.Add(Row);
	int32 InsertIndex = INDEX_NONE;
	bool bSeenSection = false;
	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		if (Entries[Index].bIsRow)
		{
			if (bSeenSection)
			{
				InsertIndex = Index + 1;
			}
			continue;
		}
		const FString Trimmed = Entries[Index].RawLine.TrimStartAndEnd();
		if (!Trimmed.StartsWith(TEXT("[")))
		{
			continue;
		}
		if (bSeenSection)
		{
			break;
		}
		if (Trimmed.Equals(SectionName, ESearchCase::IgnoreCase))
		{
			bSeenSection = true;
			InsertIndex = Index + 1;
		}
	}
	if (!bSeenSection)
	{
		FEntry Header;
		Header.RawLine = SectionName;
		Entries.Add(Header);
		InsertIndex = Entries.Num();
	}
	FEntry Entry;
	Entry.bIsRow = true;
	Entry.Row = Row;
	Entries.Insert(Entry, InsertIndex);
}

void FAccessTransformerDocument::RemoveRow(const TSharedPtr<FAccessTransformerEditRow>& Row)
{
	Entries.RemoveAll([&Row](const FEntry& Entry)
	{
		return Entry.bIsRow && Entry.Row == Row;
	});
	Rows.Remove(Row);
}

void FAccessTransformerDocument::Validate()
{
	InvalidCount = 0;
	WarningCount = 0;
	CppRebuildCount = 0;
	TSet<FString> Seen;
	for (const TSharedPtr<FAccessTransformerEditRow>& Row : Rows)
	{
		if (!Row.IsValid())
		{
			continue;
		}
		FAccessTransformerRules::ValidateRow(*Row);
		const FString Identity = FAccessTransformerRules::MakeIdentity(*Row);
		if (Seen.Contains(Identity))
		{
			Row->bValid = false;
			Row->bWarning = false;
			Row->bDuplicate = true;
			Row->ValidationMessage = TEXT("Duplicate transformer");
		}
		Seen.Add(Identity);
		InvalidCount += Row->bValid ? 0 : 1;
		WarningCount += Row->bValid && Row->bWarning ? 1 : 0;
		CppRebuildCount += FAccessTransformerRules::KindNeedsCppRebuild(Row->Kind) ? 1 : 0;
	}
}

bool FAccessTransformerDocument::CanonicalizeRuntimeTargets()
{
	bool bChanged = false;
	for (const TSharedPtr<FAccessTransformerEditRow>& Row : Rows)
	{
		if (!Row.IsValid() || !FAccessTransformerRules::IsRuntimeKind(Row->Kind))
		{
			continue;
		}
		if (UStruct* Target = FAccessTransformerRules::ResolveTarget(Row->Target))
		{
			const FString PathName = Target->GetPathName();
			if (Row->Target != PathName)
			{
				Row->Target = PathName;
				bChanged = true;
			}
		}
	}
	return bChanged;
}

int32 FAccessTransformerDocument::RemoveDuplicateRows()
{
	const int32 Before = Rows.Num();
	Entries.RemoveAll([](const FEntry& Entry)
	{
		return Entry.bIsRow && Entry.Row.IsValid() && Entry.Row->bDuplicate;
	});
	Rows.RemoveAll([](const TSharedPtr<FAccessTransformerEditRow>& Row)
	{
		return Row.IsValid() && Row->bDuplicate;
	});
	return Before - Rows.Num();
}

void FAccessTransformerDocument::MarkSaved()
{
	for (const TSharedPtr<FAccessTransformerEditRow>& Row : Rows)
	{
		if (Row.IsValid())
		{
			Row->bPendingSave = false;
		}
	}
}
