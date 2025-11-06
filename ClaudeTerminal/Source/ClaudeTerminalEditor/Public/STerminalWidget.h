// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "ClaudeAPIClient.h"
#include "EnhancedCommandExecutor.h"
#include "SceneContextBuilder.h"

/**
 * Slate widget for the Claude Terminal UI
 * Now with comprehensive NLP control for all UE5 operations
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
	TSharedPtr<FEnhancedCommandExecutor> CommandExecutor; // Now using enhanced executor
	TSharedPtr<FSceneContextBuilder> SceneContextBuilder;

	// Terminal state
	FText OutputText;
	TArray<FString> CommandHistory;
	int32 HistoryIndex;

	// ===== COMMAND HISTORY SEARCH (TIER 1.5) =====
	TArray<FString> FavoriteCommands;
	bool bHistorySearchActive;
	FString HistorySearchQuery;
	TArray<FString> FilteredHistory;
	int32 FilteredHistoryIndex;

	// Callbacks
	FReply OnSendCommand();
	void OnInputTextChanged(const FText& NewText);
	void OnInputTextCommitted(const FText& NewText, ETextCommit::Type CommitType);
	FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);

	// Claude API response handler
	void OnClaudeResponseReceived(bool bSuccess, const FString& Response);

	// History search methods
	void ActivateHistorySearch();
	void DeactivateHistorySearch();
	void UpdateHistorySearch(const FString& Query);
	void SelectNextHistoryMatch();
	void SelectPreviousHistoryMatch();
	void AddToFavorites(const FString& Command);
	void RemoveFromFavorites(const FString& Command);
	bool IsFavorite(const FString& Command) const;
	void LoadFavorites();
	void SaveFavorites();

	// Helper functions
	void AppendOutput(const FString& Text, const FLinearColor& Color = FLinearColor::White);
	void ClearOutput();
	UWorld* GetEditorWorld() const;
};
