// Copyright Epic Games, Inc. All Rights Reserved.

#include "SProgressIndicatorWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Styling/AppStyle.h"
#include "Widgets/Notifications/SProgressBar.h"

void SProgressIndicatorWidget::Construct(const FArguments& InArgs)
{
	OperationName = InArgs._OperationName;
	bShowCancelButton = InArgs._ShowCancelButton;
	bShowTimeRemaining = InArgs._ShowTimeRemaining;
	OnCancelClicked = InArgs._OnCancelClicked;

	CurrentProgress = 0.0f;
	CurrentItemIndex = 0;
	TotalItemCount = 0;
	bIsComplete = false;
	bWasCancelled = false;
	bHasError = false;
	EstimatedTimeRemaining = 0.0f;
	OperationStartTime = FPlatformTime::Seconds();
	LastUpdateTime = OperationStartTime;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(10.0f)
		[
			SNew(SVerticalBox)

			// Operation name
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 5)
			[
				SNew(STextBlock)
				.Text(FText::FromString(OperationName))
				.Font(FAppStyle::GetFontStyle("NormalFontBold"))
			]

			// Status message
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 5)
			[
				SNew(STextBlock)
				.Text(this, &SProgressIndicatorWidget::GetStatusText)
				.Font(FAppStyle::GetFontStyle("SmallFont"))
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]

			// Progress bar
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 5)
			[
				SNew(SProgressBar)
				.Percent(this, &SProgressIndicatorWidget::GetProgressBarPercent)
				.FillColorAndOpacity(this, &SProgressIndicatorWidget::GetProgressBarColor())
			]

			// Progress percentage and time remaining
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 5)
			[
				SNew(SHorizontalBox)

				// Progress percentage
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(STextBlock)
					.Text(this, &SProgressIndicatorWidget::GetProgressText)
					.Font(FAppStyle::GetFontStyle("SmallFont"))
				]

				// Time remaining
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(this, &SProgressIndicatorWidget::GetTimeRemainingText)
					.Font(FAppStyle::GetFontStyle("SmallFont"))
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
					.Visibility(this, &SProgressIndicatorWidget::GetTimeRemainingVisibility)
				]
			]

			// Cancel button
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Cancel")))
				.OnClicked(this, &SProgressIndicatorWidget::OnCancelButtonClicked)
				.Visibility(this, &SProgressIndicatorWidget::GetCancelButtonVisibility)
			]
		]
	];
}

void SProgressIndicatorWidget::UpdateProgress(float Progress, const FString& StatusMessage)
{
	CurrentProgress = FMath::Clamp(Progress, 0.0f, 1.0f);

	if (!StatusMessage.IsEmpty())
	{
		CurrentStatusMessage = StatusMessage;
	}

	UpdateTimeEstimation();
}

void SProgressIndicatorWidget::UpdateProgress(int32 CurrentItem, int32 TotalItems, const FString& StatusMessage)
{
	CurrentItemIndex = CurrentItem;
	TotalItemCount = TotalItems;

	if (TotalItems > 0)
	{
		CurrentProgress = (float)CurrentItem / (float)TotalItems;
	}
	else
	{
		CurrentProgress = 0.0f;
	}

	if (!StatusMessage.IsEmpty())
	{
		CurrentStatusMessage = StatusMessage;
	}
	else
	{
		// Generate default status message
		CurrentStatusMessage = FString::Printf(TEXT("Processing %d of %d..."), CurrentItem, TotalItems);
	}

	UpdateTimeEstimation();
}

void SProgressIndicatorWidget::SetComplete(const FString& SuccessMessage)
{
	bIsComplete = true;
	CurrentProgress = 1.0f;
	CurrentStatusMessage = SuccessMessage.IsEmpty() ? TEXT("✓ Complete!") : SuccessMessage;
}

void SProgressIndicatorWidget::SetError(const FString& ErrorMessage)
{
	bHasError = true;
	bIsComplete = true;
	CurrentStatusMessage = FString::Printf(TEXT("✗ Error: %s"), *ErrorMessage);
}

void SProgressIndicatorWidget::Reset()
{
	CurrentProgress = 0.0f;
	CurrentItemIndex = 0;
	TotalItemCount = 0;
	bIsComplete = false;
	bWasCancelled = false;
	bHasError = false;
	EstimatedTimeRemaining = 0.0f;
	OperationStartTime = FPlatformTime::Seconds();
	LastUpdateTime = OperationStartTime;
	CurrentStatusMessage.Empty();
}

FText SProgressIndicatorWidget::GetProgressText() const
{
	if (TotalItemCount > 0)
	{
		return FText::FromString(FString::Printf(TEXT("%d / %d (%.0f%%)"),
			CurrentItemIndex, TotalItemCount, CurrentProgress * 100.0f));
	}
	else
	{
		return FText::FromString(FString::Printf(TEXT("%.0f%%"), CurrentProgress * 100.0f));
	}
}

FText SProgressIndicatorWidget::GetStatusText() const
{
	return FText::FromString(CurrentStatusMessage);
}

FText SProgressIndicatorWidget::GetTimeRemainingText() const
{
	if (bIsComplete || CurrentProgress <= 0.0f)
	{
		return FText::GetEmpty();
	}

	if (EstimatedTimeRemaining < 1.0f)
	{
		return FText::FromString(TEXT("< 1 second remaining"));
	}
	else if (EstimatedTimeRemaining < 60.0f)
	{
		return FText::FromString(FString::Printf(TEXT("%.0f seconds remaining"), EstimatedTimeRemaining));
	}
	else if (EstimatedTimeRemaining < 3600.0f)
	{
		int32 Minutes = FMath::CeilToInt(EstimatedTimeRemaining / 60.0f);
		return FText::FromString(FString::Printf(TEXT("%d minutes remaining"), Minutes));
	}
	else
	{
		int32 Hours = FMath::CeilToInt(EstimatedTimeRemaining / 3600.0f);
		return FText::FromString(FString::Printf(TEXT("%d hours remaining"), Hours));
	}
}

TOptionalSize<float> SProgressIndicatorWidget::GetProgressBarPercent() const
{
	return CurrentProgress;
}

FSlateColor SProgressIndicatorWidget::GetProgressBarColor() const
{
	if (bHasError)
	{
		return FSlateColor(FLinearColor(0.8f, 0.1f, 0.1f)); // Red for errors
	}
	else if (bIsComplete)
	{
		return FSlateColor(FLinearColor(0.1f, 0.8f, 0.1f)); // Green for complete
	}
	else if (bWasCancelled)
	{
		return FSlateColor(FLinearColor(0.8f, 0.6f, 0.1f)); // Orange for cancelled
	}
	else
	{
		return FSlateColor(FLinearColor(0.2f, 0.6f, 1.0f)); // Blue for in progress
	}
}

EVisibility SProgressIndicatorWidget::GetCancelButtonVisibility() const
{
	return (bShowCancelButton && !bIsComplete) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SProgressIndicatorWidget::GetTimeRemainingVisibility() const
{
	return (bShowTimeRemaining && !bIsComplete && CurrentProgress > 0.01f) ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply SProgressIndicatorWidget::OnCancelButtonClicked()
{
	bWasCancelled = true;
	bIsComplete = true;
	CurrentStatusMessage = TEXT("Cancelled by user");

	// Execute callback if bound
	if (OnCancelClicked.IsBound())
	{
		OnCancelClicked.Execute();
	}

	return FReply::Handled();
}

void SProgressIndicatorWidget::UpdateTimeEstimation()
{
	if (CurrentProgress <= 0.0f || bIsComplete)
	{
		EstimatedTimeRemaining = 0.0f;
		return;
	}

	double CurrentTime = FPlatformTime::Seconds();
	double ElapsedTime = CurrentTime - OperationStartTime;

	// Estimate total time based on current progress
	double EstimatedTotalTime = ElapsedTime / CurrentProgress;
	EstimatedTimeRemaining = (float)(EstimatedTotalTime - ElapsedTime);

	// Clamp to reasonable values
	EstimatedTimeRemaining = FMath::Clamp(EstimatedTimeRemaining, 0.0f, 86400.0f); // Max 24 hours

	LastUpdateTime = CurrentTime;
}
