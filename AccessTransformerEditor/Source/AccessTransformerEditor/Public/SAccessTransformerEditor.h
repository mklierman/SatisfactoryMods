#pragma once

#include "AccessTransformerDocument.h"
#include "AccessTransformerTypes.h"
#include "CoreMinimal.h"
#include "Interfaces/IPluginManager.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class SAccessTransformerListRow;

class SAccessTransformerEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAccessTransformerEditor) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	bool CanCloseTab();

private:
	friend class SAccessTransformerListRow;

	TSharedRef<SWidget> BuildToolbar();
	TSharedRef<SWidget> BuildRowList();
	TSharedRef<SWidget> BuildDetails();
	TSharedRef<ITableRow> GenerateRow(TSharedPtr<FAccessTransformerEditRow> Item, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<SWidget> GenerateCell(TSharedPtr<FAccessTransformerEditRow> Item, const FName& ColumnName) const;

	void RefreshPluginList();
	void SelectPlugin(const FString& PluginName);
	void RevertPluginCombo();
	void LoadSelectedPlugin();
	void MarkDirty();
	bool Save();
	void ReloadFromDisk();
	void AddRow();
	void RemoveSelectedRow();
	void RebuildMemberOptions();
	void SyncDetailWidgets();
	void SyncKindCombo();
	void SyncMemberCombo();
	void OnSelectedRowChanged(TSharedPtr<FAccessTransformerEditRow> NewSelection, ESelectInfo::Type SelectInfo);
	void OnKindChanged(TSharedPtr<FString> NewKind, ESelectInfo::Type SelectInfo);
	void OnMemberPicked(TSharedPtr<FString> NewMember, ESelectInfo::Type SelectInfo);
	void PickClass(bool bForFriendClass);
	void PickStruct();
	void ApplyPickedStruct(UStruct* Struct, bool bForFriendClass);

	FText GetSelectedPluginText() const;
	FText GetSelectedKindText() const;
	FText GetMemberComboText() const;
	FText GetFilePathText() const;
	FText GetStatusText() const;
	FText GetValidationText() const;
	FSlateColor GetValidationColor() const;
	FText GetRowStatusText(const FAccessTransformerEditRow& Row) const;
	FSlateColor GetRowStatusColor(const FAccessTransformerEditRow& Row) const;
	FText GetMemberFieldLabel() const;
	EVisibility GetMemberFieldVisibility() const;
	bool CanEditDetails() const;

	TSharedPtr<IPlugin> GetSelectedPlugin() const;
	FString GetSelectedIniPath() const;

	TArray<TSharedPtr<FString>> PluginNames;
	TSharedPtr<FString> SelectedPluginName;
	FAccessTransformerDocument Document;
	TSharedPtr<FAccessTransformerEditRow> SelectedRow;
	TSharedPtr<SListView<TSharedPtr<FAccessTransformerEditRow>>> RowListView;
	TArray<TSharedPtr<FString>> KindOptions;
	TArray<TSharedPtr<FString>> MemberOptions;
	TSharedPtr<class SSearchableComboBox> MemberCombo;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> PluginCombo;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> KindCombo;
	TSharedPtr<SEditableTextBox> TargetTextBox;
	TSharedPtr<SMultiLineEditableTextBox> CommentTextBox;
	bool bDirty = false;
	bool bFileExists = false;
	bool bLoadFailed = false;
	bool bSuppressPluginSelection = false;
};
