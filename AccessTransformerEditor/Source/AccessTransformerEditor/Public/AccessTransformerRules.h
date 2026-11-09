#pragma once

#include "AccessTransformerTypes.h"

class UStruct;

/** Stateless conversion, reflection, and validation rules for access transformers. */
class FAccessTransformerRules
{
public:
	static bool KindFromString(const FString& Name, EAccessTransformerKind& OutKind);
	static FString KindToKey(EAccessTransformerKind Kind);
	static FText KindToText(EAccessTransformerKind Kind);
	static bool KindNeedsCppRebuild(EAccessTransformerKind Kind);
	static bool IsRuntimeKind(EAccessTransformerKind Kind);
	static bool IsPrebuiltEngineType(const UStruct* Struct);
	static bool IsPickableStruct(const UStruct* Struct, bool bAllowEngine);
	static FString FormatTarget(UStruct* Struct, EAccessTransformerKind Kind);
	static UStruct* ResolveTarget(const FString& Target);
	static FString MakeIdentity(const FAccessTransformerEditRow& Row);
	static void ValidateRow(FAccessTransformerEditRow& Row);
	static void GetMemberNames(const FAccessTransformerEditRow& Row, TArray<FString>& OutNames);
	static FString FormatIniLine(const FAccessTransformerEditRow& Row);
};
