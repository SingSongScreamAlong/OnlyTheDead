// Copyright Only The Dead Development Team. All Rights Reserved.

#include "VerdunAIAgent.h"
#include "VerdunAIAgentStyle.h"
#include "VerdunAIAgentCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "AIAgentEditorWidget.h"

static const FName VerdunAIAgentTabName("VerdunAIAgent");

#define LOCTEXT_NAMESPACE "FVerdunAIAgentModule"

void FVerdunAIAgentModule::StartupModule()
{
	// Initialize style
	FVerdunAIAgentStyle::Initialize();
	FVerdunAIAgentStyle::ReloadTextures();

	// Initialize commands
	FVerdunAIAgentCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FVerdunAIAgentCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FVerdunAIAgentModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FVerdunAIAgentModule::RegisterMenus));

	// Register tab spawner
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(VerdunAIAgentTabName, FOnSpawnTab::CreateLambda([](const FSpawnTabArgs& Args)
		{
			return SNew(SDockTab)
				.TabRole(ETabRole::NomadTab)
				[
					SNew(SAIAgentEditorWidget)
				];
		}))
		.SetDisplayName(LOCTEXT("FVerdunAIAgentTabTitle", "Verdun AI Agent"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	UE_LOG(LogTemp, Warning, TEXT("VerdunAIAgent Editor Module Started - AI Agent Ready"));
}

void FVerdunAIAgentModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FVerdunAIAgentStyle::Shutdown();
	FVerdunAIAgentCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(VerdunAIAgentTabName);

	UE_LOG(LogTemp, Log, TEXT("VerdunAIAgent Editor Module Shutdown"));
}

void FVerdunAIAgentModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(VerdunAIAgentTabName);
}

void FVerdunAIAgentModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FVerdunAIAgentCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FVerdunAIAgentCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVerdunAIAgentModule, VerdunAIAgent)
