#pragma once

#include "AccessTransformerTypes.h"

/** Parsed AccessTransformers.ini document which preserves unrelated lines and comments. */
class FAccessTransformerDocument
{
public:
	void Reset();
	bool LoadFromFile(const FString& Path);
	bool SaveToFile(const FString& Path) const;
	FString BuildIniText() const;

	void AddRow(const TSharedPtr<FAccessTransformerEditRow>& Row);
	void RemoveRow(const TSharedPtr<FAccessTransformerEditRow>& Row);
	int32 RemoveDuplicateRows();
	bool CanonicalizeRuntimeTargets();
	void Validate();
	void MarkSaved();

	TArray<TSharedPtr<FAccessTransformerEditRow>>& GetRows() { return Rows; }
	const TArray<TSharedPtr<FAccessTransformerEditRow>>& GetRows() const { return Rows; }
	int32 GetInvalidCount() const { return InvalidCount; }
	int32 GetWarningCount() const { return WarningCount; }
	int32 GetCppRebuildCount() const { return CppRebuildCount; }

private:
	struct FEntry
	{
		bool bIsRow = false;
		FString RawLine;
		TSharedPtr<FAccessTransformerEditRow> Row;
	};

	static bool ParseStructValue(const FString& Value, TMap<FString, FString>& OutFields);
	static FString CommentBody(const FString& TrimmedLine);

	TArray<FEntry> Entries;
	TArray<TSharedPtr<FAccessTransformerEditRow>> Rows;
	int32 InvalidCount = 0;
	int32 WarningCount = 0;
	int32 CppRebuildCount = 0;
};
