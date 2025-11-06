// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "ClaudeAPIClient.h"
#include "CommandExecutor.h"
#include "SceneContextBuilder.h"

/**
 * Slate widget for the Claude Terminal UI
 */
class STerminalWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STerminalWidget) {}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	/** Destructor */
	virtual ~STerminalWidget();

private:
	// UI Components
	TSharedPtr<SMultiLineEditableTextBox> OutputTextBox;
	TSharedPtr<SEditableTextBox> InputTextBox;
	TSharedPtr<SVerticalBox> MainContainer;

	// Backend components
	TSharedPtr<FClaudeAPIClient> APIClient;
	TSharedPtr<FCommandExecutor> CommandExecutor;
	TSharedPtr<FSceneContextBuilder> SceneContextBuilder;

	// Terminal state
	FText OutputText;
	TArray<FString> CommandHistory;
	int32 HistoryIndex;

	// Callbacks
	FReply OnSendCommand();
	void OnInputTextChanged(const FText& NewText);
	void OnInputTextCommitted(const FText& NewText, ETextCommit::Type CommitType);
	FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);

	// Claude API response handler
	void OnClaudeResponseReceived(bool bSuccess, const FString& Response);

	// Helper functions
	void AppendOutput(const FString& Text, const FLinearColor& Color = FLinearColor::White);
	void ClearOutput();
	UWorld* GetEditorWorld() const;
};
