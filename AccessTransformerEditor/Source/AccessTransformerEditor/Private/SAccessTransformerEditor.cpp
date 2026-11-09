#include "SAccessTransformerEditor.h"

#include "AccessTransformerEditor.h"
#include "AccessTransformerRules.h"
#include "NativeStructPicker.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/MessageDialog.h"
#include "SSearchableComboBox.h"
#include "Styling/AppStyle.h"
#include "UObject/UObjectIterator.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "AccessTransformerEditor"

namespace AccessTransformerEditorPrivate
{
	const TCHAR* IniFileName = TEXT("AccessTransformers.ini");

	FString GetIniPath(IPlugin& Plugin)
	{
		return Plugin.GetBaseDir() / TEXT("Config") / IniFileName;
	}

	void ShowNotification(const FText& Message, bool bSuccess)
	{
		FNotificationInfo Info(Message);
		Info.ExpireDuration = 4.0f;
		TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
		if (Notification.IsValid())
		{
			Notification->SetCompletionState(bSuccess ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
		}
	}
} // namespace AccessTransformerEditorPrivate

class SAccessTransformerListRow : public SMultiColumnTableRow<TSharedPtr<FAccessTransformerEditRow>>
{
public:
	SLATE_BEGIN_ARGS(SAccessTransformerListRow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments&, const TSharedRef<STableViewBase>& OwnerTable, TSharedPtr<FAccessTransformerEditRow> InItem, SAccessTransformerEditor* InEditor)
	{
		Item = InItem;
		Editor = InEditor;
		SMultiColumnTableRow<TSharedPtr<FAccessTransformerEditRow>>::Construct(FSuperRowType::FArguments(), OwnerTable);
	}

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		if (!Editor || !Item.IsValid())
		{
			return SNullWidget::NullWidget;
		}
		return Editor->GenerateCell(Item, ColumnName);
	}

private:
	TSharedPtr<FAccessTransformerEditRow> Item;
	SAccessTransformerEditor* Editor = nullptr;
};

void SAccessTransformerEditor::Construct(const FArguments& InArgs)
{
	KindOptions = {
		MakeShared<FString>(TEXT("Friend")),
		MakeShared<FString>(TEXT("Accessor")),
		MakeShared<FString>(TEXT("BlueprintReadWrite")),
		MakeShared<FString>(TEXT("BlueprintCallable")),
		MakeShared<FString>(TEXT("EditAnywhere")),
		MakeShared<FString>(TEXT("BlueprintType")),
	};

	RefreshPluginList();

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(8.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				BuildToolbar()
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
			[
				SNew(STextBlock)
				.Text(this, &SAccessTransformerEditor::GetFilePathText)
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
			[
				SNew(STextBlock)
				.Text(this, &SAccessTransformerEditor::GetStatusText)
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().FillHeight(1.0f)
			[
				SNew(SSplitter)
				.Orientation(Orient_Vertical)
				+ SSplitter::Slot().Value(0.55f)
				[
					BuildRowList()
				]
				+ SSplitter::Slot().Value(0.45f)
				[
					BuildDetails()
				]
			]
		]
	];

	if (PluginNames.Num() > 0)
	{
		FString Initial = *PluginNames[0];
		for (const TSharedPtr<FString>& Name : PluginNames)
		{
			TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(*Name);
			if (Plugin.IsValid() && FPaths::FileExists(AccessTransformerEditorPrivate::GetIniPath(*Plugin)))
			{
				Initial = *Name;
				break;
			}
		}
		SelectPlugin(Initial);
	}
}

TSharedRef<SWidget> SAccessTransformerEditor::BuildToolbar()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(0.45f).VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
		[
			SAssignNew(PluginCombo, SComboBox<TSharedPtr<FString>>)
			.OptionsSource(&PluginNames)
			.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
			{
				return SNew(STextBlock).Text(FText::FromString(Item.IsValid() ? *Item : FString()));
			})
			.OnSelectionChanged_Lambda([this](TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo)
			{
				if (bSuppressPluginSelection || SelectInfo == ESelectInfo::Direct || !Item.IsValid())
				{
					return;
				}
				SelectPlugin(*Item);
			})
			[
				SNew(STextBlock).Text(this, &SAccessTransformerEditor::GetSelectedPluginText)
			]
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("AddRow", "Add"))
			.IsEnabled_Lambda([this]()
			{
				return SelectedPluginName.IsValid() && !bLoadFailed;
			})
			.OnClicked_Lambda([this]()
			{
				AddRow();
				return FReply::Handled();
			})
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("RemoveRow", "Remove"))
			.OnClicked_Lambda([this]()
			{
				RemoveSelectedRow();
				return FReply::Handled();
			})
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Reload", "Reload"))
			.OnClicked_Lambda([this]()
			{
				ReloadFromDisk();
				return FReply::Handled();
			})
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Save", "Save"))
			.IsEnabled_Lambda([this]()
			{
				return SelectedPluginName.IsValid() && !bLoadFailed;
			})
			.OnClicked_Lambda([this]()
			{
				Save();
				return FReply::Handled();
			})
		];
}

TSharedRef<SWidget> SAccessTransformerEditor::BuildRowList()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
		[
			SAssignNew(RowListView, SListView<TSharedPtr<FAccessTransformerEditRow>>)
			.ListItemsSource(&Document.GetRows())
			.OnGenerateRow(this, &SAccessTransformerEditor::GenerateRow)
			.OnSelectionChanged(this, &SAccessTransformerEditor::OnSelectedRowChanged)
			.HeaderRow(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("Kind").DefaultLabel(LOCTEXT("ColKind", "Type")).FillWidth(0.22f)
				+ SHeaderRow::Column("Target").DefaultLabel(LOCTEXT("ColTarget", "Class / Struct")).FillWidth(0.38f)
				+ SHeaderRow::Column("Member").DefaultLabel(LOCTEXT("ColMember", "Member")).FillWidth(0.28f)
				+ SHeaderRow::Column("Status").DefaultLabel(LOCTEXT("ColStatus", "Status")).FillWidth(0.12f)
			)
		];
}

TSharedRef<ITableRow> SAccessTransformerEditor::GenerateRow(TSharedPtr<FAccessTransformerEditRow> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SAccessTransformerListRow, OwnerTable, Item, this);
}

TSharedRef<SWidget> SAccessTransformerEditor::GenerateCell(TSharedPtr<FAccessTransformerEditRow> Item, const FName& ColumnName) const
{
	if (!Item.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	if (ColumnName != "Kind" && ColumnName != "Target" && ColumnName != "Member" && ColumnName != "Status")
	{
		return SNullWidget::NullWidget;
	}

	return SNew(SBox)
		.Padding(FMargin(4.0f, 2.0f))
		[
			SNew(STextBlock)
			.Text_Lambda([this, Item, ColumnName]()
			{
				if (ColumnName == "Kind")
				{
					return FAccessTransformerRules::KindToText(Item->Kind);
				}
				if (ColumnName == "Target")
				{
					return FText::FromString(Item->Target);
				}
				if (ColumnName == "Member")
				{
					return FText::FromString(Item->Member);
				}
				return GetRowStatusText(*Item);
			})
			.ColorAndOpacity_Lambda([this, Item, ColumnName]()
			{
				return ColumnName == "Status" ? GetRowStatusColor(*Item) : FSlateColor::UseForeground();
			})
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
		];
}

TSharedRef<SWidget> SAccessTransformerEditor::BuildDetails()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(8.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("DetailsHeader", "Selected transformer"))
					.Font(FAppStyle::GetFontStyle("NormalFontBold"))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
					[
						SNew(STextBlock).Text(LOCTEXT("TypeLabel", "Type"))
					]
					+ SHorizontalBox::Slot().FillWidth(1.0f)
					[
						SAssignNew(KindCombo, SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&KindOptions)
						.IsEnabled(this, &SAccessTransformerEditor::CanEditDetails)
						.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
						{
							return SNew(STextBlock).Text(FText::FromString(Item.IsValid() ? *Item : FString()));
						})
						.OnSelectionChanged(this, &SAccessTransformerEditor::OnKindChanged)
						[
							SNew(STextBlock).Text(this, &SAccessTransformerEditor::GetSelectedKindText)
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
				[
					SNew(STextBlock).Text(LOCTEXT("TargetLabel", "Class / Struct"))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.0f)
					[
						SAssignNew(TargetTextBox, SEditableTextBox)
						.IsEnabled(this, &SAccessTransformerEditor::CanEditDetails)
						.OnTextCommitted_Lambda([this](const FText& NewText, ETextCommit::Type)
						{
							if (!SelectedRow.IsValid())
							{
								return;
							}
							const FString NewTarget = NewText.ToString().TrimStartAndEnd();
							if (SelectedRow->Target == NewTarget)
							{
								return;
							}
							SelectedRow->Target = NewTarget;
							if (TargetTextBox.IsValid())
							{
								TargetTextBox->SetText(FText::FromString(NewTarget));
							}
							RebuildMemberOptions();
							SyncMemberCombo();
							MarkDirty();
						})
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f, 0.0f, 0.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("PickClass", "Class"))
						.IsEnabled(this, &SAccessTransformerEditor::CanEditDetails)
						.OnClicked_Lambda([this]()
						{
							PickClass(false);
							return FReply::Handled();
						})
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f, 0.0f, 0.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("PickStruct", "Struct"))
						.IsEnabled(this, &SAccessTransformerEditor::CanEditDetails)
						.OnClicked_Lambda([this]()
						{
							PickStruct();
							return FReply::Handled();
						})
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
				[
					SNew(SVerticalBox)
					.Visibility(this, &SAccessTransformerEditor::GetMemberFieldVisibility)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
					[
						SNew(STextBlock).Text(this, &SAccessTransformerEditor::GetMemberFieldLabel)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
					[
						SNew(SButton)
						.Text_Lambda([this]()
						{
							return SelectedRow.IsValid() && !SelectedRow->Member.IsEmpty()
								? FText::FromString(SelectedRow->Member)
								: LOCTEXT("PickFriendClass", "Pick friend class...");
						})
						.IsEnabled(this, &SAccessTransformerEditor::CanEditDetails)
						.Visibility_Lambda([this]()
						{
							return SelectedRow.IsValid() && SelectedRow->Kind == EAccessTransformerKind::Friend
								? EVisibility::Visible
								: EVisibility::Collapsed;
						})
						.OnClicked_Lambda([this]()
						{
							PickClass(true);
							return FReply::Handled();
						})
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SAssignNew(MemberCombo, SSearchableComboBox)
						.IsEnabled(this, &SAccessTransformerEditor::CanEditDetails)
						.OptionsSource(&MemberOptions)
						.bAlwaysSelectItem(true)
						.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
						{
							return SNew(STextBlock).Text(FText::FromString(Item.IsValid() ? *Item : FString()));
						})
						.OnSelectionChanged(this, &SAccessTransformerEditor::OnMemberPicked)
						.Visibility_Lambda([this]()
						{
							return SelectedRow.IsValid() && SelectedRow->Kind != EAccessTransformerKind::Friend
								? EVisibility::Visible
								: EVisibility::Collapsed;
						})
						[
							SNew(STextBlock).Text(this, &SAccessTransformerEditor::GetMemberComboText)
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
				[
					SNew(STextBlock).Text(LOCTEXT("CommentLabel", "Comment"))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
				[
					SAssignNew(CommentTextBox, SMultiLineEditableTextBox)
					.IsEnabled(this, &SAccessTransformerEditor::CanEditDetails)
					.AutoWrapText(true)
					.OnTextCommitted_Lambda([this](const FText& NewText, ETextCommit::Type)
					{
						if (!SelectedRow.IsValid())
						{
							return;
						}
						const FString NewComment = NewText.ToString().TrimStartAndEnd();
						if (SelectedRow->Comment == NewComment)
						{
							return;
						}
						SelectedRow->Comment = NewComment;
						MarkDirty();
					})
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(this, &SAccessTransformerEditor::GetValidationText)
					.ColorAndOpacity(this, &SAccessTransformerEditor::GetValidationColor)
				]
			]
		];
}

void SAccessTransformerEditor::RefreshPluginList()
{
	PluginNames.Reset();
	TArray<FString> Names;
	FString ModsDirectory = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("Mods"));
	FPaths::NormalizeDirectoryName(ModsDirectory);
	ModsDirectory += TEXT("/");
	for (const TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetEnabledPlugins())
	{
		FString PluginDirectory = FPaths::ConvertRelativePathToFull(Plugin->GetBaseDir());
		FPaths::NormalizeDirectoryName(PluginDirectory);
		PluginDirectory += TEXT("/");
		if (!PluginDirectory.StartsWith(ModsDirectory, ESearchCase::IgnoreCase))
		{
			continue;
		}
		if (Plugin->GetName() == TEXT("AccessTransformerEditor"))
		{
			continue;
		}
		Names.Add(Plugin->GetName());
	}
	Names.Sort();
	for (const FString& Name : Names)
	{
		PluginNames.Add(MakeShared<FString>(Name));
	}
}

void SAccessTransformerEditor::SelectPlugin(const FString& PluginName)
{
	if (bSuppressPluginSelection)
	{
		return;
	}
	if (SelectedPluginName.IsValid() && *SelectedPluginName == PluginName)
	{
		return;
	}

	if (bDirty && SelectedPluginName.IsValid())
	{
		const EAppReturnType::Type Choice = FMessageDialog::Open(
			EAppMsgType::YesNoCancel,
			LOCTEXT("UnsavedChanges", "Save changes to AccessTransformers.ini before switching mods?"));
		if (Choice == EAppReturnType::Cancel || (Choice == EAppReturnType::Yes && !Save()))
		{
			RevertPluginCombo();
			return;
		}
	}

	SelectedPluginName.Reset();
	for (const TSharedPtr<FString>& Name : PluginNames)
	{
		if (Name.IsValid() && *Name == PluginName)
		{
			SelectedPluginName = Name;
			break;
		}
	}
	if (!SelectedPluginName.IsValid())
	{
		SelectedPluginName = MakeShared<FString>(PluginName);
	}

	if (PluginCombo.IsValid())
	{
		bSuppressPluginSelection = true;
		PluginCombo->SetSelectedItem(SelectedPluginName);
		bSuppressPluginSelection = false;
	}
	LoadSelectedPlugin();
}

void SAccessTransformerEditor::RevertPluginCombo()
{
	if (!PluginCombo.IsValid())
	{
		return;
	}
	bSuppressPluginSelection = true;
	PluginCombo->SetSelectedItem(SelectedPluginName);
	bSuppressPluginSelection = false;
}

bool SAccessTransformerEditor::CanCloseTab()
{
	if (!bDirty)
	{
		return true;
	}
	const EAppReturnType::Type Choice = FMessageDialog::Open(
		EAppMsgType::YesNoCancel,
		LOCTEXT("UnsavedClose", "Save changes to AccessTransformers.ini before closing?"));
	if (Choice == EAppReturnType::Cancel)
	{
		return false;
	}
	if (Choice == EAppReturnType::Yes)
	{
		return Save();
	}
	return true;
}

void SAccessTransformerEditor::LoadSelectedPlugin()
{
	Document.Reset();
	SelectedRow.Reset();
	bDirty = false;
	bFileExists = false;
	bLoadFailed = false;

	TSharedPtr<IPlugin> Plugin = GetSelectedPlugin();
	if (Plugin.IsValid())
	{
		const FString Path = AccessTransformerEditorPrivate::GetIniPath(*Plugin);
		bFileExists = FPaths::FileExists(Path);
		if (bFileExists && !Document.LoadFromFile(Path))
		{
			bLoadFailed = true;
			Document.Reset();
			UE_LOG(LogAccessTransformerEditor, Error, TEXT("Failed to read %s"), *Path);
			AccessTransformerEditorPrivate::ShowNotification(FText::Format(
																 LOCTEXT("LoadFailed", "Failed to read {0}. Saving is disabled so the file is not overwritten."),
																 FText::FromString(Path)),
															 false);
		}
	}

	Document.Validate();
	if (RowListView.IsValid())
	{
		RowListView->RequestListRefresh();
		if (Document.GetRows().Num() > 0)
		{
			RowListView->SetSelection(Document.GetRows()[0], ESelectInfo::Direct);
		}
		else
		{
			RebuildMemberOptions();
			SyncDetailWidgets();
		}
	}
}

void SAccessTransformerEditor::MarkDirty()
{
	bDirty = true;
	if (SelectedRow.IsValid())
	{
		SelectedRow->bPendingSave = true;
	}
	Document.Validate();
	if (RowListView.IsValid())
	{
		RowListView->RequestListRefresh();
	}
}

bool SAccessTransformerEditor::Save()
{
	TSharedPtr<IPlugin> Plugin = GetSelectedPlugin();
	if (!Plugin.IsValid() || bLoadFailed)
	{
		return false;
	}

	const bool bCanonicalized = Document.CanonicalizeRuntimeTargets();
	Document.Validate();
	const bool bSelectedDuplicate = SelectedRow.IsValid() && SelectedRow->bDuplicate;
	const int32 DuplicateCount = Document.RemoveDuplicateRows();
	if (bCanonicalized || DuplicateCount > 0)
	{
		bDirty = true;
	}
	Document.Validate();
	if (bSelectedDuplicate)
	{
		SelectedRow.Reset();
		if (RowListView.IsValid() && Document.GetRows().Num() > 0)
		{
			RowListView->SetSelection(Document.GetRows()[0], ESelectInfo::Direct);
		}
	}

	const FString Path = AccessTransformerEditorPrivate::GetIniPath(*Plugin);
	if (!Document.SaveToFile(Path))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(
												  LOCTEXT("SaveFailed", "Failed to write {0}"),
												  FText::FromString(Path)));
		SyncDetailWidgets();
		if (RowListView.IsValid())
		{
			RowListView->RequestListRefresh();
		}
		return false;
	}

	bDirty = false;
	bFileExists = true;
	Document.MarkSaved();
	UE_LOG(LogAccessTransformerEditor, Display, TEXT("Wrote %s"), *Path);
	SyncDetailWidgets();
	if (RowListView.IsValid())
	{
		RowListView->RequestListRefresh();
	}

	FText Message;
	if (Document.GetInvalidCount() > 0)
	{
		Message = FText::Format(
			LOCTEXT("SavedWithErrors", "Wrote {0} with {1} validation error(s)."),
			FText::FromString(FPaths::GetCleanFilename(Path)),
			FText::AsNumber(Document.GetInvalidCount()));
	}
	else if (Document.GetCppRebuildCount() > 0)
	{
		Message = FText::Format(
			LOCTEXT("SavedNeedsCpp", "Wrote {0}. Friend/Accessor rows need a C++ rebuild."),
			FText::FromString(FPaths::GetCleanFilename(Path)));
	}
	else
	{
		Message = FText::Format(
			LOCTEXT("SavedOk", "Wrote {0}"),
			FText::FromString(FPaths::GetCleanFilename(Path)));
	}
	if (DuplicateCount > 0)
	{
		Message = FText::Format(
			LOCTEXT("SavedDroppedDupes", "{0} Removed {1} duplicate row(s)."),
			Message,
			FText::AsNumber(DuplicateCount));
	}

	AccessTransformerEditorPrivate::ShowNotification(Message, Document.GetInvalidCount() == 0);
	return true;
}

void SAccessTransformerEditor::ReloadFromDisk()
{
	if (bDirty)
	{
		const EAppReturnType::Type Choice = FMessageDialog::Open(
			EAppMsgType::YesNo,
			LOCTEXT("DiscardChanges", "Reload from disk and discard unsaved changes?"));
		if (Choice != EAppReturnType::Yes)
		{
			return;
		}
	}
	LoadSelectedPlugin();
}

void SAccessTransformerEditor::AddRow()
{
	if (bLoadFailed || !SelectedPluginName.IsValid())
	{
		return;
	}
	TSharedPtr<FAccessTransformerEditRow> Row = MakeShared<FAccessTransformerEditRow>();
	Row->bPendingSave = true;
	if (SelectedRow.IsValid())
	{
		Row->Kind = SelectedRow->Kind;
		Row->Target = SelectedRow->Target;
	}
	Document.AddRow(Row);
	SelectedRow = Row;
	MarkDirty();
	if (RowListView.IsValid())
	{
		RowListView->SetSelection(Row, ESelectInfo::OnMouseClick);
	}
}

void SAccessTransformerEditor::RemoveSelectedRow()
{
	if (!SelectedRow.IsValid())
	{
		return;
	}
	const TSharedPtr<FAccessTransformerEditRow> Removing = SelectedRow;
	Document.RemoveRow(Removing);
	SelectedRow.Reset();
	MarkDirty();
	if (Document.GetRows().Num() > 0 && RowListView.IsValid())
	{
		RowListView->SetSelection(Document.GetRows()[0], ESelectInfo::Direct);
	}
	else
	{
		RebuildMemberOptions();
		SyncDetailWidgets();
	}
}

void SAccessTransformerEditor::RebuildMemberOptions()
{
	MemberOptions.Reset();
	if (!SelectedRow.IsValid() || SelectedRow->Kind == EAccessTransformerKind::BlueprintType || SelectedRow->Kind == EAccessTransformerKind::Friend)
	{
		if (MemberCombo.IsValid())
		{
			MemberCombo->RefreshOptions();
		}
		return;
	}

	TArray<FString> Sorted;
	FAccessTransformerRules::GetMemberNames(*SelectedRow, Sorted);
	for (const FString& Name : Sorted)
	{
		MemberOptions.Add(MakeShared<FString>(Name));
	}
	if (MemberCombo.IsValid())
	{
		MemberCombo->RefreshOptions();
	}
}

void SAccessTransformerEditor::SyncDetailWidgets()
{
	const FString Target = SelectedRow.IsValid() ? SelectedRow->Target : FString();
	const FString Comment = SelectedRow.IsValid() ? SelectedRow->Comment : FString();
	if (TargetTextBox.IsValid())
	{
		TargetTextBox->SetText(FText::FromString(Target));
	}
	if (CommentTextBox.IsValid())
	{
		CommentTextBox->SetText(FText::FromString(Comment));
	}
	SyncKindCombo();
	SyncMemberCombo();
}

void SAccessTransformerEditor::SyncKindCombo()
{
	if (!KindCombo.IsValid())
	{
		return;
	}
	if (!SelectedRow.IsValid())
	{
		KindCombo->ClearSelection();
		return;
	}
	const FString Key = FAccessTransformerRules::KindToKey(SelectedRow->Kind);
	for (const TSharedPtr<FString>& Option : KindOptions)
	{
		if (Option.IsValid() && *Option == Key)
		{
			KindCombo->SetSelectedItem(Option);
			return;
		}
	}
}

void SAccessTransformerEditor::SyncMemberCombo()
{
	if (!MemberCombo.IsValid())
	{
		return;
	}
	MemberCombo->ClearSelection();
	if (!SelectedRow.IsValid())
	{
		return;
	}
	for (const TSharedPtr<FString>& Option : MemberOptions)
	{
		if (Option.IsValid() && *Option == SelectedRow->Member)
		{
			MemberCombo->SetSelectedItem(Option, ESelectInfo::Direct);
			return;
		}
	}
}

void SAccessTransformerEditor::OnSelectedRowChanged(TSharedPtr<FAccessTransformerEditRow> NewSelection, ESelectInfo::Type SelectInfo)
{
	SelectedRow = NewSelection;
	RebuildMemberOptions();
	SyncDetailWidgets();
}

void SAccessTransformerEditor::OnKindChanged(TSharedPtr<FString> NewKind, ESelectInfo::Type SelectInfo)
{
	if (!SelectedRow.IsValid() || !NewKind.IsValid() || SelectInfo == ESelectInfo::Direct || SelectInfo == ESelectInfo::OnNavigation)
	{
		return;
	}
	EAccessTransformerKind Kind;
	if (!FAccessTransformerRules::KindFromString(*NewKind, Kind) || SelectedRow->Kind == Kind)
	{
		return;
	}

	SelectedRow->Kind = Kind;
	SelectedRow->Target.Reset();
	SelectedRow->Member.Reset();
	SelectedRow->Comment.Reset();
	RebuildMemberOptions();
	SyncDetailWidgets();
	MarkDirty();
}

void SAccessTransformerEditor::OnMemberPicked(TSharedPtr<FString> NewMember, ESelectInfo::Type SelectInfo)
{
	if (!SelectedRow.IsValid() || !NewMember.IsValid() || SelectInfo == ESelectInfo::Direct || SelectInfo == ESelectInfo::OnNavigation)
	{
		return;
	}
	if (SelectedRow->Member == *NewMember)
	{
		return;
	}
	SelectedRow->Member = *NewMember;
	MarkDirty();
}

void SAccessTransformerEditor::PickClass(bool bForFriendClass)
{
	if (!SelectedRow.IsValid())
	{
		return;
	}

	TSet<FString> FriendClassPackages;
	if (bForFriendClass)
	{
		const TSharedPtr<IPlugin> Plugin = GetSelectedPlugin();
		if (!Plugin.IsValid())
		{
			return;
		}
		for (const FModuleDescriptor& Module : Plugin->GetDescriptor().Modules)
		{
			FriendClassPackages.Add(FString::Printf(TEXT("/Script/%s"), *Module.Name.ToString()));
		}
	}

	const bool bAllowEngine = bForFriendClass || FAccessTransformerRules::IsRuntimeKind(SelectedRow->Kind);
	TArray<UStruct*> Candidates;
	for (TObjectIterator<UClass> It; It; ++It)
	{
		const bool bInPickerScope = bForFriendClass
										? FriendClassPackages.Contains((*It)->GetOutermost()->GetName())
										: (*It)->GetPathName().StartsWith(TEXT("/Script/FactoryGame."));
		if (bInPickerScope && FAccessTransformerRules::IsPickableStruct(*It, bAllowEngine))
		{
			Candidates.Add(*It);
		}
	}

	if (UStruct* Picked = FNativeStructPicker::Pick(
			bForFriendClass ? LOCTEXT("PickFriendTitle", "Pick friend class") : LOCTEXT("PickClassTitle", "Pick native class"),
			Candidates,
			true))
	{
		ApplyPickedStruct(Picked, bForFriendClass);
	}
}

void SAccessTransformerEditor::PickStruct()
{
	if (!SelectedRow.IsValid())
	{
		return;
	}

	const bool bAllowEngine = FAccessTransformerRules::IsRuntimeKind(SelectedRow->Kind);
	TArray<UStruct*> Candidates;
	for (TObjectIterator<UScriptStruct> It; It; ++It)
	{
		if (FAccessTransformerRules::IsPickableStruct(*It, bAllowEngine))
		{
			Candidates.Add(*It);
		}
	}

	if (UStruct* Picked = FNativeStructPicker::Pick(LOCTEXT("PickStructTitle", "Pick native struct"), Candidates))
	{
		ApplyPickedStruct(Picked, false);
	}
}

void SAccessTransformerEditor::ApplyPickedStruct(UStruct* Struct, bool bForFriendClass)
{
	if (!SelectedRow.IsValid() || !Struct)
	{
		return;
	}

	if (bForFriendClass)
	{
		if (const UClass* Class = Cast<UClass>(Struct))
		{
			SelectedRow->Member = FString::Printf(TEXT("%s%s"), Class->GetPrefixCPP(), *Class->GetName());
		}
		else
		{
			SelectedRow->Member = FAccessTransformerRules::FormatTarget(Struct, EAccessTransformerKind::Friend);
		}
	}
	else
	{
		SelectedRow->Target = FAccessTransformerRules::FormatTarget(Struct, SelectedRow->Kind);
	}

	RebuildMemberOptions();
	SyncDetailWidgets();
	MarkDirty();
}

FText SAccessTransformerEditor::GetSelectedPluginText() const
{
	return SelectedPluginName.IsValid() ? FText::FromString(*SelectedPluginName) : LOCTEXT("NoPlugin", "Select a mod");
}

FText SAccessTransformerEditor::GetSelectedKindText() const
{
	return SelectedRow.IsValid() ? FAccessTransformerRules::KindToText(SelectedRow->Kind) : FText();
}

FText SAccessTransformerEditor::GetMemberComboText() const
{
	if (SelectedRow.IsValid() && !SelectedRow->Member.IsEmpty())
	{
		return FText::FromString(SelectedRow->Member);
	}
	return LOCTEXT("PickMember", "Pick from class...");
}

FText SAccessTransformerEditor::GetFilePathText() const
{
	const FString Path = GetSelectedIniPath();
	return Path.IsEmpty() ? FText() : FText::FromString(Path);
}

FText SAccessTransformerEditor::GetStatusText() const
{
	if (!SelectedPluginName.IsValid())
	{
		return LOCTEXT("NoPluginStatus", "Select a mod to edit its AccessTransformers.ini");
	}
	if (bLoadFailed)
	{
		return LOCTEXT("LoadFailedStatus", "Could not read AccessTransformers.ini. Saving is disabled so the file is not overwritten.");
	}

	const FText State = bDirty
							? LOCTEXT("Dirty", "Unsaved")
							: (bFileExists ? LOCTEXT("Clean", "Saved") : LOCTEXT("NoFile", "No file"));
	return FText::Format(
		LOCTEXT("StatusFmt", "{0}  |  {1} transformer(s)  |  {2} error(s)  |  {3} warning(s){4}"),
		State,
		FText::AsNumber(Document.GetRows().Num()),
		FText::AsNumber(Document.GetInvalidCount()),
		FText::AsNumber(Document.GetWarningCount()),
		Document.GetCppRebuildCount() > 0 ? LOCTEXT("CppNote", "  |  Friend/Accessor need C++ rebuild") : FText());
}

FText SAccessTransformerEditor::GetValidationText() const
{
	if (!SelectedRow.IsValid())
	{
		return LOCTEXT("NoSelection", "Select a row to edit it.");
	}
	return FText::FromString(SelectedRow->ValidationMessage);
}

FSlateColor SAccessTransformerEditor::GetValidationColor() const
{
	if (!SelectedRow.IsValid())
	{
		return FSlateColor::UseForeground();
	}
	if (!SelectedRow->bValid)
	{
		return FSlateColor(FLinearColor(1.0f, 0.35f, 0.35f));
	}
	if (SelectedRow->bWarning)
	{
		return FSlateColor(FLinearColor(1.0f, 0.8f, 0.2f));
	}
	return FSlateColor::UseForeground();
}

FText SAccessTransformerEditor::GetRowStatusText(const FAccessTransformerEditRow& Row) const
{
	if (!Row.bValid)
	{
		return LOCTEXT("Invalid", "Error");
	}
	if (Row.bWarning)
	{
		return LOCTEXT("Warn", "Warn");
	}
	if (Row.bPendingSave)
	{
		return LOCTEXT("PendingSave", "Pending");
	}
	if (FAccessTransformerRules::KindNeedsCppRebuild(Row.Kind))
	{
		return LOCTEXT("NeedsRebuild", "C++");
	}
	return LOCTEXT("Live", "Live");
}

FSlateColor SAccessTransformerEditor::GetRowStatusColor(const FAccessTransformerEditRow& Row) const
{
	if (!Row.bValid)
	{
		return FSlateColor(FLinearColor(1.0f, 0.3f, 0.3f));
	}
	if (Row.bWarning || Row.bPendingSave || FAccessTransformerRules::KindNeedsCppRebuild(Row.Kind))
	{
		return FSlateColor(FLinearColor(1.0f, 0.8f, 0.2f));
	}
	return FSlateColor(FLinearColor(0.4f, 0.8f, 0.4f));
}

FText SAccessTransformerEditor::GetMemberFieldLabel() const
{
	if (!SelectedRow.IsValid())
	{
		return LOCTEXT("MemberLabel", "Member");
	}
	switch (SelectedRow->Kind)
	{
	case EAccessTransformerKind::Friend:
		return LOCTEXT("FriendClassLabel", "Friend class");
	case EAccessTransformerKind::BlueprintCallable:
		return LOCTEXT("FunctionLabel", "Function");
	default:
		return LOCTEXT("PropertyLabel", "Property");
	}
}

EVisibility SAccessTransformerEditor::GetMemberFieldVisibility() const
{
	return SelectedRow.IsValid() && SelectedRow->Kind != EAccessTransformerKind::BlueprintType
			   ? EVisibility::Visible
			   : EVisibility::Collapsed;
}

bool SAccessTransformerEditor::CanEditDetails() const
{
	return SelectedRow.IsValid() && !bLoadFailed;
}

TSharedPtr<IPlugin> SAccessTransformerEditor::GetSelectedPlugin() const
{
	if (!SelectedPluginName.IsValid())
	{
		return nullptr;
	}
	return IPluginManager::Get().FindPlugin(*SelectedPluginName);
}

FString SAccessTransformerEditor::GetSelectedIniPath() const
{
	TSharedPtr<IPlugin> Plugin = GetSelectedPlugin();
	return Plugin.IsValid() ? AccessTransformerEditorPrivate::GetIniPath(*Plugin) : FString();
}

#undef LOCTEXT_NAMESPACE
