#include "NativeStructPicker.h"

#include "Framework/Application/SlateApplication.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "AccessTransformerEditor"

UStruct* FNativeStructPicker::Pick(const FText& Title, const TArray<UStruct*>& Candidates, bool bUseShortNames)
{
	struct FPickState
	{
		TArray<TSharedPtr<FString>> AllLabels;
		TArray<TSharedPtr<FString>> Filtered;
		TMap<FString, UStruct*> ByLabel;
		TSharedPtr<FString> Picked;
		TSharedPtr<SListView<TSharedPtr<FString>>> List;
		FString Query;
		bool bAccepted = false;
	};
	TSharedRef<FPickState> State = MakeShared<FPickState>();
	for (UStruct* Struct : Candidates)
	{
		if (!Struct)
		{
			continue;
		}
		const FString LabelText = bUseShortNames ? Struct->GetName() : Struct->GetPathName();
		if (State->ByLabel.Contains(LabelText))
		{
			continue;
		}
		State->AllLabels.Add(MakeShared<FString>(LabelText));
		State->ByLabel.Add(LabelText, Struct);
	}
	State->AllLabels.Sort([](const TSharedPtr<FString>& A, const TSharedPtr<FString>& B)
	{
		return *A < *B;
	});
	State->Filtered = State->AllLabels;
	TSharedPtr<SWindow> Window = SNew(SWindow)
		.Title(Title)
		.ClientSize(FVector2D(640.0, 560.0))
		.SupportsMinimize(false)
		.SupportsMaximize(false);
	auto ApplyFilter = [State]()
	{
		State->Filtered.Reset();
		for (const TSharedPtr<FString>& Label : State->AllLabels)
		{
			if (State->Query.IsEmpty() || (Label.IsValid() && Label->Contains(State->Query, ESearchCase::IgnoreCase)))
			{
				State->Filtered.Add(Label);
			}
		}
		if (State->List.IsValid())
		{
			State->List->RequestListRefresh();
		}
	};
	Window->SetContent(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)
		[
			SNew(SEditableTextBox)
			.HintText(LOCTEXT("SymbolSearch", "Search"))
			.OnTextChanged_Lambda([State, ApplyFilter](const FText& NewText)
			{
				State->Query = NewText.ToString();
				ApplyFilter();
			})
		]
		+ SVerticalBox::Slot().FillHeight(1.0f).Padding(8.0f, 0.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
			[
				SAssignNew(State->List, SListView<TSharedPtr<FString>>)
				.ListItemsSource(&State->Filtered)
				.SelectionMode(ESelectionMode::Single)
				.OnSelectionChanged_Lambda([State](TSharedPtr<FString> Item, ESelectInfo::Type)
				{
					State->Picked = Item;
				})
				.OnMouseButtonDoubleClick_Lambda([State, Window](TSharedPtr<FString> Item)
				{
					if (Item.IsValid())
					{
						State->Picked = Item;
						State->bAccepted = true;
						Window->RequestDestroyWindow();
					}
				})
				.OnGenerateRow_Lambda([](TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable)
				{
					return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
					[
						SNew(STextBlock)
						.Text(FText::FromString(Item.IsValid() ? *Item : FString()))
						.Margin(FMargin(4.0f, 2.0f))
					];
				})
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(8.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("SymbolOk", "OK"))
				.IsEnabled_Lambda([State]()
				{
					return State->Picked.IsValid();
				})
				.OnClicked_Lambda([State, Window]()
				{
					State->bAccepted = true;
					Window->RequestDestroyWindow();
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SymbolCancel", "Cancel"))
				.OnClicked_Lambda([Window]()
				{
					Window->RequestDestroyWindow();
					return FReply::Handled();
				})
			]
		]
	);
	FSlateApplication::Get().AddModalWindow(Window.ToSharedRef(), FSlateApplication::Get().GetActiveTopLevelWindow());
	if (!State->bAccepted || !State->Picked.IsValid())
	{
		return nullptr;
	}
	UStruct** Found = State->ByLabel.Find(*State->Picked);
	return Found ? *Found : nullptr;
}

#undef LOCTEXT_NAMESPACE
