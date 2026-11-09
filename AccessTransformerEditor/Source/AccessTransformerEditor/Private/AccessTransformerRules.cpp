#include "AccessTransformerRules.h"

#include "UObject/Package.h"
#include "UObject/UnrealType.h"

bool FAccessTransformerRules::KindFromString(const FString& Name, EAccessTransformerKind& OutKind)
{
	if (Name.Equals(TEXT("Friend"), ESearchCase::IgnoreCase))
	{
		OutKind = EAccessTransformerKind::Friend;
		return true;
	}
	if (Name.Equals(TEXT("Accessor"), ESearchCase::IgnoreCase))
	{
		OutKind = EAccessTransformerKind::Accessor;
		return true;
	}
	if (Name.Equals(TEXT("BlueprintReadWrite"), ESearchCase::IgnoreCase))
	{
		OutKind = EAccessTransformerKind::BlueprintReadWrite;
		return true;
	}
	if (Name.Equals(TEXT("BlueprintCallable"), ESearchCase::IgnoreCase))
	{
		OutKind = EAccessTransformerKind::BlueprintCallable;
		return true;
	}
	if (Name.Equals(TEXT("EditAnywhere"), ESearchCase::IgnoreCase))
	{
		OutKind = EAccessTransformerKind::EditAnywhere;
		return true;
	}
	if (Name.Equals(TEXT("BlueprintType"), ESearchCase::IgnoreCase))
	{
		OutKind = EAccessTransformerKind::BlueprintType;
		return true;
	}
	return false;
}

FString FAccessTransformerRules::KindToKey(EAccessTransformerKind Kind)
{
	switch (Kind)
	{
	case EAccessTransformerKind::Friend:
		return TEXT("Friend");
	case EAccessTransformerKind::Accessor:
		return TEXT("Accessor");
	case EAccessTransformerKind::BlueprintReadWrite:
		return TEXT("BlueprintReadWrite");
	case EAccessTransformerKind::BlueprintCallable:
		return TEXT("BlueprintCallable");
	case EAccessTransformerKind::EditAnywhere:
		return TEXT("EditAnywhere");
	case EAccessTransformerKind::BlueprintType:
		return TEXT("BlueprintType");
	default:
		return TEXT("BlueprintReadWrite");
	}
}

FText FAccessTransformerRules::KindToText(EAccessTransformerKind Kind)
{
	return FText::FromString(KindToKey(Kind));
}

bool FAccessTransformerRules::KindNeedsCppRebuild(EAccessTransformerKind Kind)
{
	return Kind == EAccessTransformerKind::Friend || Kind == EAccessTransformerKind::Accessor;
}

bool FAccessTransformerRules::IsRuntimeKind(EAccessTransformerKind Kind)
{
	return !KindNeedsCppRebuild(Kind);
}

bool FAccessTransformerRules::IsPrebuiltEngineType(const UStruct* Struct)
{
	if (!Struct)
	{
		return false;
	}
	const FString PathName = Struct->GetPathName();
	return PathName.StartsWith(TEXT("/Script/Engine.")) || PathName.StartsWith(TEXT("/Script/CoreUObject."));
}

bool FAccessTransformerRules::IsPickableStruct(const UStruct* Struct, bool bAllowEngine)
{
	if (!Struct || !Struct->IsNative() || Struct->HasAnyFlags(RF_NewerVersionExists) || Struct->GetOutermost() == GetTransientPackage())
	{
		return false;
	}
	if (const UClass* Class = Cast<UClass>(Struct))
	{
		if (!Class->HasAnyClassFlags(CLASS_Native) || Class->HasAnyClassFlags(CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_CompiledFromBlueprint))
		{
			return false;
		}
	}
	if (!bAllowEngine && IsPrebuiltEngineType(Struct))
	{
		return false;
	}
	const FString Name = Struct->GetName();
	return !Name.StartsWith(TEXT("SKEL_")) && !Name.StartsWith(TEXT("REINST_")) && !Name.StartsWith(TEXT("TRASHCLASS_")) && !Name.Contains(TEXT("REINST_"));
}

FString FAccessTransformerRules::FormatTarget(UStruct* Struct, EAccessTransformerKind Kind)
{
	if (!Struct)
	{
		return FString();
	}
	if (KindNeedsCppRebuild(Kind))
	{
		if (const UClass* Class = Cast<UClass>(Struct))
		{
			return FString::Printf(TEXT("%s%s"), Class->GetPrefixCPP(), *Class->GetName());
		}
		return FString::Printf(TEXT("F%s"), *Struct->GetName());
	}
	return Struct->GetPathName();
}

UStruct* FAccessTransformerRules::ResolveTarget(const FString& Target)
{
	if (Target.IsEmpty())
	{
		return nullptr;
	}
	if (UStruct* Exact = FindObject<UStruct>(nullptr, *Target))
	{
		return Exact;
	}
	if (UStruct* ByName = FindFirstObject<UStruct>(*Target, EFindFirstObjectOptions::NativeFirst, ELogVerbosity::NoLogging, TEXT("AccessTransformerEditor")))
	{
		return ByName;
	}
	if (Target.Len() > 1)
	{
		return FindFirstObject<UStruct>(*Target.RightChop(1), EFindFirstObjectOptions::NativeFirst, ELogVerbosity::NoLogging, TEXT("AccessTransformerEditor"));
	}
	return nullptr;
}

FString FAccessTransformerRules::MakeIdentity(const FAccessTransformerEditRow& Row)
{
	FString TargetId = Row.Target;
	if (UStruct* Resolved = ResolveTarget(Row.Target))
	{
		TargetId = IsRuntimeKind(Row.Kind) ? Resolved->GetPathName() : FormatTarget(Resolved, Row.Kind);
	}
	return KindToKey(Row.Kind) + TEXT("|") + TargetId + TEXT("|") + Row.Member;
}

void FAccessTransformerRules::ValidateRow(FAccessTransformerEditRow& Row)
{
	Row.bValid = true;
	Row.bWarning = false;
	Row.bDuplicate = false;
	Row.ValidationMessage.Reset();
	if (Row.Target.IsEmpty())
	{
		Row.bValid = false;
		Row.ValidationMessage = TEXT("Class / struct is required");
		return;
	}
	UStruct* Target = ResolveTarget(Row.Target);
	if (!Target)
	{
		Row.bValid = false;
		Row.ValidationMessage = FString::Printf(TEXT("Could not find %s"), *Row.Target);
		return;
	}
	const FString PathName = Target->GetPathName();
	if (KindNeedsCppRebuild(Row.Kind) && IsPrebuiltEngineType(Target))
	{
		Row.bValid = false;
		Row.ValidationMessage = TEXT("Friend and Accessor cannot be applied to a prebuilt engine type");
		return;
	}
	FString Warning;
	if (IsRuntimeKind(Row.Kind) && Row.Target != PathName)
	{
		Warning = FString::Printf(TEXT("Unscoped name. Saving rewrites this to %s."), *PathName);
	}
	if (Row.Kind != EAccessTransformerKind::BlueprintType && Row.Member.IsEmpty())
	{
		Row.bValid = false;
		Row.ValidationMessage = TEXT("Member is required");
		return;
	}
	if (Row.Kind == EAccessTransformerKind::BlueprintReadWrite || Row.Kind == EAccessTransformerKind::EditAnywhere || Row.Kind == EAccessTransformerKind::Accessor)
	{
		FProperty* Property = FindFProperty<FProperty>(Target, *Row.Member);
		if (!Property)
		{
			Row.bValid = false;
			Row.ValidationMessage = FString::Printf(TEXT("Could not find property %s:%s"), *PathName, *Row.Member);
			return;
		}
		if (Row.Kind == EAccessTransformerKind::Accessor && Property->GetOwnerStruct() != Target)
		{
			Row.bValid = false;
			Row.ValidationMessage = TEXT("Accessor only applies to a property declared on this type");
			return;
		}
	}
	else if (Row.Kind == EAccessTransformerKind::BlueprintCallable && !FindUField<UFunction>(Target, *Row.Member))
	{
		Row.bValid = false;
		Row.ValidationMessage = FString::Printf(TEXT("Could not find function %s:%s"), *PathName, *Row.Member);
		return;
	}
	if (!Warning.IsEmpty())
	{
		Row.bWarning = true;
		Row.ValidationMessage = Warning;
	}
	else if (KindNeedsCppRebuild(Row.Kind))
	{
		Row.ValidationMessage = TEXT("Applies after a C++ rebuild");
	}
	else
	{
		Row.ValidationMessage = TEXT("Applies when AccessTransformers.ini is saved");
	}
}

void FAccessTransformerRules::GetMemberNames(const FAccessTransformerEditRow& Row, TArray<FString>& OutNames)
{
	OutNames.Reset();
	if (Row.Kind == EAccessTransformerKind::BlueprintType || Row.Kind == EAccessTransformerKind::Friend)
	{
		return;
	}
	TSet<FString> Unique;
	if (UStruct* Target = ResolveTarget(Row.Target))
	{
		if (Row.Kind == EAccessTransformerKind::BlueprintCallable)
		{
			for (TFieldIterator<UFunction> It(Target); It; ++It)
			{
				Unique.Add(It->GetName());
			}
		}
		else
		{
			const EFieldIterationFlags Flags = Row.Kind == EAccessTransformerKind::Accessor ? EFieldIterationFlags::IncludeDeprecated : EFieldIterationFlags::Default;
			for (TFieldIterator<FProperty> It(Target, Flags); It; ++It)
			{
				Unique.Add(It->GetName());
			}
		}
	}
	OutNames = Unique.Array();
	OutNames.Sort();
}

FString FAccessTransformerRules::FormatIniLine(const FAccessTransformerEditRow& Row)
{
	auto Quote = [](const FString& Value)
	{
		return FString::Printf(TEXT("\"%s\""), *Value);
	};
	FString Value;
	if (Row.Kind == EAccessTransformerKind::Friend)
	{
		Value = FString::Printf(TEXT("(Class=%s, FriendClass=%s)"), *Quote(Row.Target), *Quote(Row.Member));
	}
	else if (Row.Kind == EAccessTransformerKind::BlueprintCallable)
	{
		Value = FString::Printf(TEXT("(Class=%s, Function=%s)"), *Quote(Row.Target), *Quote(Row.Member));
	}
	else if (Row.Kind == EAccessTransformerKind::BlueprintType)
	{
		Value = FString::Printf(TEXT("(Struct=%s)"), *Quote(Row.Target));
	}
	else
	{
		Value = FString::Printf(TEXT("(Class=%s, Property=%s)"), *Quote(Row.Target), *Quote(Row.Member));
	}
	return KindToKey(Row.Kind) + TEXT("=") + Value;
}
