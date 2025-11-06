// Copyright Epic Games, Inc. All Rights Reserved.

#include "ClaudeTerminalEditor.h"
#include "STerminalWidget.h"
#include "CoordinatePickerMode.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "EditorModeManager.h"
#include "EditorModeRegistry.h"

static const FName ClaudeTerminalTabName("ClaudeTerminal");

#define LOCTEXT_NAMESPACE "FClaudeTerminalEditorModule"

void FClaudeTerminalEditorModule::StartupModule()
{
	// Register coordinate picker editor mode (TIER 2.1)
	FEditorModeRegistry::Get().RegisterMode<FCoordinatePickerMode>(
		FCoordinatePickerMode::EM_CoordinatePicker,
		LOCTEXT("CoordinatePickerModeName", "Coordinate Picker"),
		FSlateIcon(),
		false);

	// Register tab spawner
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		ClaudeTerminalTabName,
		FOnSpawnTab::CreateRaw(this, &FClaudeTerminalEditorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("ClaudeTerminalTabTitle", "Claude Terminal"))
		.SetMenuType(ETabSpawnerMenuType::Hidden)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));

	// Register menus
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FClaudeTerminalEditorModule::RegisterMenus));
}

void FClaudeTerminalEditorModule::ShutdownModule()
{
	// Unregister coordinate picker editor mode
	FEditorModeRegistry::Get().UnregisterMode(FCoordinatePickerMode::EM_CoordinatePicker);

	// Unregister tab spawner
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ClaudeTerminalTabName);

	// Unregister menus
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
}

TSharedRef<SDockTab> FClaudeTerminalEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(STerminalWidget)
		];
}

void FClaudeTerminalEditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ClaudeTerminalTabName);
}

void FClaudeTerminalEditorModule::RegisterMenus()
{
	// Register the menu entry
	FToolMenuOwnerScoped OwnerScoped(this);

	// Add to Window menu
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
	if (Menu)
	{
		FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
		Section.AddMenuEntryWithCommandList(
			FName("ClaudeTerminal"),
			LOCTEXT("ClaudeTerminalMenuEntry", "Claude AI Terminal"),
			LOCTEXT("ClaudeTerminalMenuEntryTooltip", "Open the Claude AI Terminal for natural language UE5 commands"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"),
			FUIAction(FExecuteAction::CreateRaw(this, &FClaudeTerminalEditorModule::PluginButtonClicked)),
			nullptr
		);
	}

	// Add toolbar button
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	if (ToolbarMenu)
	{
		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("ClaudeTerminal");
		Section.AddEntry(
			FToolMenuEntry::InitToolBarButton(
				FName("ClaudeTerminal"),
				FUIAction(FExecuteAction::CreateRaw(this, &FClaudeTerminalEditorModule::PluginButtonClicked)),
				LOCTEXT("ClaudeTerminalToolbarButton", "Claude Terminal"),
				LOCTEXT("ClaudeTerminalToolbarButtonTooltip", "Open the Claude AI Terminal"),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports")
			)
		);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FClaudeTerminalEditorModule, ClaudeTerminalEditor)
