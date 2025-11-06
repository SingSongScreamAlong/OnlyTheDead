// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ClaudeTerminalSettings.generated.h"

/**
 * Settings for Claude Terminal Plugin
 * Accessible via Project Settings > Plugins > Claude Terminal
 */
UCLASS(Config=EditorPerProjectUserSettings, DefaultConfig, meta=(DisplayName="Claude Terminal"))
class CLAUDETERMINAL_API UClaudeTerminalSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UClaudeTerminalSettings();

	/** Claude API Key - Get yours at https://console.anthropic.com/ */
	UPROPERTY(Config, EditAnywhere, Category="API", meta=(DisplayName="API Key"))
	FString APIKey;

	/** Claude model to use */
	UPROPERTY(Config, EditAnywhere, Category="API", meta=(DisplayName="Model Name"))
	FString ModelName;

	/** Maximum tokens per response */
	UPROPERTY(Config, EditAnywhere, Category="API", meta=(DisplayName="Max Tokens", ClampMin="1", ClampMax="8192"))
	int32 MaxTokens;

	/** API endpoint URL */
	UPROPERTY(Config, EditAnywhere, Category="API|Advanced", meta=(DisplayName="API Endpoint"))
	FString APIEndpoint;

	/** Enable command history */
	UPROPERTY(Config, EditAnywhere, Category="Terminal", meta=(DisplayName="Enable Command History"))
	bool bEnableCommandHistory;

	/** Maximum number of commands to keep in history */
	UPROPERTY(Config, EditAnywhere, Category="Terminal", meta=(DisplayName="Max History Size", ClampMin="10", ClampMax="1000"))
	int32 MaxHistorySize;

	/** Auto-scroll output to bottom */
	UPROPERTY(Config, EditAnywhere, Category="Terminal|UI", meta=(DisplayName="Auto Scroll"))
	bool bAutoScroll;

	/** Terminal font size */
	UPROPERTY(Config, EditAnywhere, Category="Terminal|UI", meta=(DisplayName="Font Size", ClampMin="8", ClampMax="24"))
	int32 FontSize;

	/** Show confirmation for destructive operations */
	UPROPERTY(Config, EditAnywhere, Category="Safety", meta=(DisplayName="Confirm Destructive Operations"))
	bool bConfirmDestructiveOps;

	/** Enable undo/redo for terminal commands */
	UPROPERTY(Config, EditAnywhere, Category="Safety", meta=(DisplayName="Enable Undo/Redo"))
	bool bEnableUndo;

	/** Send scene context to Claude automatically */
	UPROPERTY(Config, EditAnywhere, Category="Context", meta=(DisplayName="Auto-Send Scene Context"))
	bool bAutoSendSceneContext;

	/** Maximum number of actors to include in scene context */
	UPROPERTY(Config, EditAnywhere, Category="Context", meta=(DisplayName="Max Context Actors", ClampMin="10", ClampMax="500"))
	int32 MaxContextActors;

	// UDeveloperSettings interface
	virtual FName GetCategoryName() const override;
	virtual FText GetSectionText() const override;

#if WITH_EDITOR
	virtual FText GetSectionDescription() const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
