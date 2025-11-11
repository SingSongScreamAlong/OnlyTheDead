// Copyright Only The Dead Development Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Views/SListView.h"
#include "AIAgentCore.h"
#include "LLMIntegration.h"

class SScrollBox;
class SButton;
class SEditableTextBox;

/**
 * Log entry for the agent's activity
 */
struct FAgentLogEntry
{
	FString Message;
	FDateTime Timestamp;
	bool bIsError;

	FAgentLogEntry(const FString& InMessage, bool bInIsError = false)
		: Message(InMessage)
		, Timestamp(FDateTime::Now())
		, bIsError(bInIsError)
	{
	}
};

/**
 * Main editor widget for the AI Agent
 */
class SAIAgentEditorWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAIAgentEditorWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual ~SAIAgentEditorWidget();

private:
	// UI Creation
	TSharedRef<SWidget> CreateConfigurationPanel();
	TSharedRef<SWidget> CreateCommandPanel();
	TSharedRef<SWidget> CreateProgressPanel();
	TSharedRef<SWidget> CreateLogPanel();

	// UI Callbacks
	FReply OnExecuteCommand();
	FReply OnStopExecution();
	FReply OnClearLog();
	void OnProviderChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);

	// Agent callbacks
	void OnAgentTaskStarted(const FString& TaskDescription);
	void OnAgentTaskCompleted(const FString& TaskDescription, bool bSuccess);
	void OnAgentThinking(const FString& ThoughtProcess, float Progress);
	void OnAgentLog(const FString& LogMessage);

	// Log management
	void AddLogEntry(const FString& Message, bool bIsError = false);
	TSharedRef<ITableRow> GenerateLogRow(TSharedPtr<FAgentLogEntry> Entry, const TSharedRef<STableViewBase>& OwnerTable);

	// UI State
	EActiveTimerReturnType UpdateUI(double InCurrentTime, float InDeltaTime);
	FText GetStatusText() const;
	FSlateColor GetStatusColor() const;

	// Member variables
	UPROPERTY()
	UAIAgentCore* AgentCore;

	// UI Elements
	TSharedPtr<SEditableTextBox> CommandInput;
	TSharedPtr<SEditableTextBox> APIKeyInput;
	TSharedPtr<SEditableTextBox> ModelNameInput;
	TSharedPtr<SEditableTextBox> ProjectContextInput;
	TSharedPtr<SButton> ExecuteButton;
	TSharedPtr<SButton> StopButton;
	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<SProgressBar> ProgressBar;
	TSharedPtr<SListView<TSharedPtr<FAgentLogEntry>>> LogListView;

	// Data
	TArray<TSharedPtr<FString>> ProviderOptions;
	TSharedPtr<FString> SelectedProvider;
	TArray<TSharedPtr<FAgentLogEntry>> LogEntries;

	FLLMRequestConfig CurrentConfig;
	float CurrentProgress;
	FString CurrentThought;
};
