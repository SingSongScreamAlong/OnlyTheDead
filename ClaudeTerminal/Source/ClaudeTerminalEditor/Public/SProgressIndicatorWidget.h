// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

/**
 * Progress indicator widget for displaying long-running operation status
 * Shows progress bar, percentage, status message, and cancel button
 */
class CLAUDETERMINALEDITOR_API SProgressIndicatorWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SProgressIndicatorWidget)
		: _OperationName(TEXT("Processing"))
		, _ShowCancelButton(true)
		, _ShowTimeRemaining(true)
		{}

		/** Name of the operation being performed */
		SLATE_ARGUMENT(FString, OperationName)

		/** Whether to show a cancel button */
		SLATE_ARGUMENT(bool, ShowCancelButton)

		/** Whether to show estimated time remaining */
		SLATE_ARGUMENT(bool, ShowTimeRemaining)

		/** Callback when cancel button is clicked */
		SLATE_EVENT(FSimpleDelegate, OnCancelClicked)

	SLATE_END_ARGS()

	/** Constructs this widget */
	void Construct(const FArguments& InArgs);

	/**
	 * Update progress (0.0 to 1.0)
	 * @param Progress Current progress (0.0 = 0%, 1.0 = 100%)
	 * @param StatusMessage Optional status message to display
	 */
	void UpdateProgress(float Progress, const FString& StatusMessage = TEXT(""));

	/**
	 * Update progress with item counts
	 * @param CurrentItem Current item being processed
	 * @param TotalItems Total number of items
	 * @param StatusMessage Optional status message
	 */
	void UpdateProgress(int32 CurrentItem, int32 TotalItems, const FString& StatusMessage = TEXT(""));

	/**
	 * Mark operation as complete
	 * @param SuccessMessage Message to show on completion
	 */
	void SetComplete(const FString& SuccessMessage = TEXT("Complete!"));

	/**
	 * Mark operation as failed
	 * @param ErrorMessage Error message to display
	 */
	void SetError(const FString& ErrorMessage);

	/**
	 * Reset progress to initial state
	 */
	void Reset();

	/**
	 * Check if operation is complete
	 */
	bool IsComplete() const { return bIsComplete; }

	/**
	 * Check if operation was cancelled
	 */
	bool WasCancelled() const { return bWasCancelled; }

	/**
	 * Get current progress (0.0 to 1.0)
	 */
	float GetProgress() const { return CurrentProgress; }

private:
	// Slate widget methods
	FText GetProgressText() const;
	FText GetStatusText() const;
	FText GetTimeRemainingText() const;
	TOptionalSize<float> GetProgressBarPercent() const;
	FSlateColor GetProgressBarColor() const;
	EVisibility GetCancelButtonVisibility() const;
	EVisibility GetTimeRemainingVisibility() const;

	// Button callbacks
	FReply OnCancelButtonClicked();

	// Update time estimation
	void UpdateTimeEstimation();

	// Operation state
	FString OperationName;
	FString CurrentStatusMessage;
	float CurrentProgress;
	int32 CurrentItemIndex;
	int32 TotalItemCount;
	bool bIsComplete;
	bool bWasCancelled;
	bool bHasError;
	bool bShowCancelButton;
	bool bShowTimeRemaining;

	// Time tracking for estimation
	double OperationStartTime;
	double LastUpdateTime;

	// Estimated time remaining in seconds
	float EstimatedTimeRemaining;

	// Delegates
	FSimpleDelegate OnCancelClicked;
};
