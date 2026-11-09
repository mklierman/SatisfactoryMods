#include "AccessTransformerEditor.h"
#include "SAccessTransformerEditor.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "AccessTransformerEditor"

DEFINE_LOG_CATEGORY(LogAccessTransformerEditor)

const FName FAccessTransformerEditorModule::TabName(TEXT("AccessTransformerEditor"));

void FAccessTransformerEditorModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		TabName,
		FOnSpawnTab::CreateLambda([](const FSpawnTabArgs&)
		{
			TSharedRef<SAccessTransformerEditor> Editor = SNew(SAccessTransformerEditor);
			return SNew(SDockTab)
				.TabRole(ETabRole::NomadTab)
				.Label(LOCTEXT("TabTitle", "Access Transformers"))
				.OnCanCloseTab(SDockTab::FCanCloseTab::CreateSP(Editor, &SAccessTransformerEditor::CanCloseTab))
				[
					Editor
				];
		}))
		.SetDisplayName(LOCTEXT("TabTitle", "Access Transformers"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	TSharedPtr<FExtender> MenuExtender = MakeShared<FExtender>();
	MenuExtender->AddMenuExtension(
		TEXT("FileProject"),
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateLambda([](FMenuBuilder& Builder)
		{
			Builder.AddMenuEntry(
				LOCTEXT("OpenEditor", "Access Transformers"),
				LOCTEXT("OpenEditorTooltip", "Edit AccessTransformers.ini for a mod"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([]()
				{
					FGlobalTabmanager::Get()->TryInvokeTab(TabName);
				}))
			);
		}));
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
}

void FAccessTransformerEditorModule::ShutdownModule()
{
	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabName);
	}
}

IMPLEMENT_MODULE(FAccessTransformerEditorModule, AccessTransformerEditor)

#undef LOCTEXT_NAMESPACE
