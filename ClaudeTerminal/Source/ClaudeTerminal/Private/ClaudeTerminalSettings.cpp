// Copyright Epic Games, Inc. All Rights Reserved.

#include "ClaudeTerminalSettings.h"

#define LOCTEXT_NAMESPACE "ClaudeTerminal"

UClaudeTerminalSettings::UClaudeTerminalSettings()
	: APIKey(TEXT(""))
	, ModelName(TEXT("claude-sonnet-4-5-20250929"))
	, MaxTokens(4096)
	, APIEndpoint(TEXT("https://api.anthropic.com/v1/messages"))
	, bEnableCommandHistory(true)
	, MaxHistorySize(100)
	, bAutoScroll(true)
	, FontSize(10)
	, bConfirmDestructiveOps(true)
	, bEnableUndo(true)
	, bAutoSendSceneContext(true)
	, MaxContextActors(100)
{
}

FName UClaudeTerminalSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}

FText UClaudeTerminalSettings::GetSectionText() const
{
	return LOCTEXT("SettingsSection", "Claude Terminal");
}

#if WITH_EDITOR
FText UClaudeTerminalSettings::GetSectionDescription() const
{
	return LOCTEXT("SettingsDescription", "Configure the Claude AI Terminal plugin settings, including API key and behavior options.");
}

void UClaudeTerminalSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		const FName PropertyName = PropertyChangedEvent.Property->GetFName();

		// Validate API key format
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UClaudeTerminalSettings, APIKey))
		{
			if (!APIKey.IsEmpty() && !APIKey.StartsWith(TEXT("sk-ant-")))
			{
				UE_LOG(LogTemp, Warning, TEXT("Claude API key should start with 'sk-ant-'. Please verify your key."));
			}
		}

		// Notify that settings changed
		SaveConfig();
	}
}
#endif

#undef LOCTEXT_NAMESPACE
