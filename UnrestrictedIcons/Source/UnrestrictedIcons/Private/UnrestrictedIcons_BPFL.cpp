#include "UnrestrictedIcons_BPFL.h"

template<typename T>
int AddValueToEnum(FName NewName)
{
	UEnum* Enum = StaticEnum<T>();
	FString EnumName = Enum->GetName();
	if (!NewName.ToString().StartsWith(EnumName))
	{
		NewName = FName(FString::Printf(TEXT("%s::%s"), *EnumName, *NewName.ToString()));
	}
	if (Enum->IsValidEnumName(NewName))
	{
		return -1;
	}
	bool HasMax = Enum->ContainsExistingMax();
	int64 NewValue = HasMax ? Enum->GetMaxEnumValue() : Enum->NumEnums();
	TArray<TPair<FName, int64>> Names;
	int EnumNum = Enum->NumEnums();
	if (HasMax)
	{
		EnumNum--;
	}
	for (int i = 0; i < EnumNum; ++i)
	{
		FName Name = Enum->GetNameByIndex(i);
		int64 Value = Enum->GetValueByIndex(i);
		Names.Emplace(TPair<FName, int64>(Name, Value));
		FString enumName = Enum->GetNameStringByIndex(i);
		FString enumIdx = FString::FromInt(Value);
	}
	auto New = TPair<FName, int64>(NewName, NewValue);
	Names.Emplace(New);
	EEnumFlags Flag = Enum->HasAnyEnumFlags(EEnumFlags::Flags) ? EEnumFlags::Flags : EEnumFlags::None;
	Enum->SetEnums(Names, Enum->GetCppForm(), Flag, HasMax);
	return EnumNum;
}

void UUnrestrictedIcons_BPFL::AddEnums()
{
	UEnum* Enum = StaticEnum<EIconType>();

	if (Enum->GetValueByNameString("ESIT_Mods") == INDEX_NONE)
	{
		AddValueToEnum<EIconType>(FName("ESIT_Mods"));
	}

	if (Enum->GetValueByNameString("ESIT_Imported") == INDEX_NONE)
	{
		AddValueToEnum<EIconType>(FName("ESIT_Imported"));
	}

	if (Enum->GetValueByNameString("ESIT_Extra") == INDEX_NONE)
	{
		AddValueToEnum<EIconType>(FName("ESIT_Extra"));
	}
}

int UUnrestrictedIcons_BPFL::GetModsCategoryEnumValue()
{
	UEnum* Enum = StaticEnum<EIconType>();
	return Enum->GetValueByNameString("EIconType::ESIT_Mods");
}

int UUnrestrictedIcons_BPFL::GetImportedCategoryEnumValue()
{
	UEnum* Enum = StaticEnum<EIconType>();
	return Enum->GetValueByNameString("EIconType::ESIT_Imported");
}

int UUnrestrictedIcons_BPFL::GetExtraCategoryEnumValue()
{
	UEnum* Enum = StaticEnum<EIconType>();
	return Enum->GetValueByNameString("EIconType::ESIT_Extra");
}