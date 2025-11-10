// Copyright Epic Games, Inc. All Rights Reserved.

#include "VerdunAutoTerrainBuilderEditor.h"
#include "VerdunTerrainBuilderSubsystem.h"
#include "ToolMenus.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "FVerdunAutoTerrainBuilderEditorModule"

void FVerdunAutoTerrainBuilderEditorModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("VerdunAutoTerrainBuilder Editor Module Started"));

	// Register menus
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FVerdunAutoTerrainBuilderEditorModule::RegisterMenus));
}

void FVerdunAutoTerrainBuilderEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	UE_LOG(LogTemp, Log, TEXT("VerdunAutoTerrainBuilder Editor Module Shutdown"));
}

void FVerdunAutoTerrainBuilderEditorModule::PluginButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Verdun Auto Terrain Builder button clicked!"));

	// Get the subsystem and trigger terrain building
	if (GEditor)
	{
		UVerdunTerrainBuilderSubsystem* Subsystem = GEditor->GetEditorSubsystem<UVerdunTerrainBuilderSubsystem>();
		if (Subsystem)
		{
			Subsystem->OpenTerrainBuilderWindow();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get VerdunTerrainBuilderSubsystem"));
		}
	}
}

void FVerdunAutoTerrainBuilderEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(
				"VerdunAutoTerrainBuilder",
				LOCTEXT("VerdunAutoTerrainBuilderLabel", "Verdun Auto Terrain Builder"),
				LOCTEXT("VerdunAutoTerrainBuilderTooltip", "Opens the Verdun Auto Terrain Builder window"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateRaw(this, &FVerdunAutoTerrainBuilderEditorModule::PluginButtonClicked))
			);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(
					"VerdunAutoTerrainBuilder",
					FUIAction(FExecuteAction::CreateRaw(this, &FVerdunAutoTerrainBuilderEditorModule::PluginButtonClicked)),
					LOCTEXT("VerdunAutoTerrainBuilderLabel", "Verdun Terrain"),
					LOCTEXT("VerdunAutoTerrainBuilderTooltip", "Build Verdun Battlefield Terrain"),
					FSlateIcon()
				));
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVerdunAutoTerrainBuilderEditorModule, VerdunAutoTerrainBuilderEditor)
