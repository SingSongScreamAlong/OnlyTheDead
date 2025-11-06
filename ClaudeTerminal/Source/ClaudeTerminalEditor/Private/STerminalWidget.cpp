// Copyright Epic Games, Inc. All Rights Reserved.

#include "STerminalWidget.h"
#include "ClaudeTerminalSettings.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SSeparator.h"
#include "EditorStyleSet.h"
#include "Editor.h"
#include "Misc/ConfigCacheIni.h"

#define LOCTEXT_NAMESPACE "ClaudeTerminal"

void STerminalWidget::Construct(const FArguments& InArgs)
{
	// Initialize backend components
	APIClient = MakeShared<FClaudeAPIClient>();
	CommandExecutor = MakeShared<FCommandExecutor>();
	SceneContextBuilder = MakeShared<FSceneContextBuilder>();

	// Initialize state
	CommandHistory.Empty();
	HistoryIndex = 0;
	OutputText = LOCTEXT("WelcomeMessage", "Claude Terminal v1.0\nType your commands below. Claude will help you manipulate the UE5 world.\n\n");

	ChildSlot
	[
		SNew(SVerticalBox)

		// Header
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("TerminalTitle", "Claude AI Terminal"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearButton", "Clear"))
				.OnClicked_Lambda([this]() -> FReply
				{
					ClearOutput();
					return FReply::Handled();
				})
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("ResetConversation", "New Conversation"))
				.OnClicked_Lambda([this]() -> FReply
				{
					APIClient->ClearConversation();
					AppendOutput("Conversation cleared. Starting fresh.\n", FLinearColor::Yellow);
					return FReply::Handled();
				})
			]
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSeparator)
		]

		// Output area
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(5.0f)
		[
			SNew(SScrollBox)
			.Orientation(Orient_Vertical)

			+ SScrollBox::Slot()
			[
				SAssignNew(OutputTextBox, SMultiLineEditableTextBox)
				.Text(OutputText)
				.IsReadOnly(true)
				.AutoWrapText(true)
				.Margin(FMargin(5.0f))
				.Font(FCoreStyle::GetDefaultFontStyle("Mono", 10))
			]
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSeparator)
		]

		// Input area
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(InputTextBox, SEditableTextBox)
				.HintText(LOCTEXT("InputHint", "Type your command here... (e.g., 'Create a road from 0,0,0 to 1000,0,0')"))
				.OnTextCommitted(this, &STerminalWidget::OnInputTextCommitted)
				.OnKeyDownHandler(this, &STerminalWidget::OnKeyDown)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("SendButton", "Send"))
				.OnClicked(this, &STerminalWidget::OnSendCommand)
			]
		]

		// Status bar
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5.0f)
		[
			SNew(STextBlock)
			.Text_Lambda([this]() -> FText
			{
				const UClaudeTerminalSettings* Settings = GetDefault<UClaudeTerminalSettings>();
				if (!Settings || Settings->APIKey.IsEmpty())
				{
					return LOCTEXT("NoAPIKey", "⚠ No API Key configured. Go to Project Settings > Plugins > Claude Terminal to set your API key.");
				}
				else
				{
					return LOCTEXT("Ready", "✓ Ready");
				}
			})
			.ColorAndOpacity_Lambda([this]() -> FSlateColor
			{
				const UClaudeTerminalSettings* Settings = GetDefault<UClaudeTerminalSettings>();
				return (!Settings || Settings->APIKey.IsEmpty()) ? FLinearColor::Red : FLinearColor::Green;
			})
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
		]
	];
}

STerminalWidget::~STerminalWidget()
{
}

FReply STerminalWidget::OnSendCommand()
{
	FString Command = InputTextBox->GetText().ToString().TrimStartAndEnd();

	if (Command.IsEmpty())
	{
		return FReply::Handled();
	}

	// Add to history
	CommandHistory.Add(Command);
	HistoryIndex = CommandHistory.Num();

	// Display user command
	AppendOutput(FString::Printf(TEXT("> %s\n"), *Command), FLinearColor(0.5f, 0.8f, 1.0f));

	// Clear input
	InputTextBox->SetText(FText::GetEmpty());

	// Build message with scene context if enabled
	const UClaudeTerminalSettings* Settings = GetDefault<UClaudeTerminalSettings>();
	FString MessageToSend = Command;

	if (Settings && Settings->bAutoSendSceneContext && SceneContextBuilder.IsValid())
	{
		UWorld* World = GetEditorWorld();
		if (World)
		{
			FString SceneContext = SceneContextBuilder->BuildContext(World, true, Settings->MaxContextActors);
			if (!SceneContext.IsEmpty())
			{
				MessageToSend = SceneContext + TEXT("\nUser Request: ") + Command;
			}
		}
	}

	// Show "thinking" indicator
	AppendOutput(TEXT("Claude is thinking...\n"), FLinearColor::Yellow);

	// Send to Claude API
	APIClient->SendMessage(
		MessageToSend,
		FClaudeAPIClient::FOnResponseReceived::CreateSP(this, &STerminalWidget::OnClaudeResponseReceived)
	);

	return FReply::Handled();
}

void STerminalWidget::OnInputTextChanged(const FText& NewText)
{
	// Handle text changes if needed
}

void STerminalWidget::OnInputTextCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter)
	{
		OnSendCommand();
	}
}

FReply STerminalWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Up)
	{
		// Navigate history backwards
		if (CommandHistory.Num() > 0 && HistoryIndex > 0)
		{
			HistoryIndex--;
			InputTextBox->SetText(FText::FromString(CommandHistory[HistoryIndex]));
		}
		return FReply::Handled();
	}
	else if (InKeyEvent.GetKey() == EKeys::Down)
	{
		// Navigate history forwards
		if (CommandHistory.Num() > 0 && HistoryIndex < CommandHistory.Num() - 1)
		{
			HistoryIndex++;
			InputTextBox->SetText(FText::FromString(CommandHistory[HistoryIndex]));
		}
		else
		{
			HistoryIndex = CommandHistory.Num();
			InputTextBox->SetText(FText::GetEmpty());
		}
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void STerminalWidget::OnClaudeResponseReceived(bool bSuccess, const FString& Response)
{
	// Remove "thinking" indicator by replacing the output text
	FString CurrentOutput = OutputText.ToString();
	int32 ThinkingIndex = CurrentOutput.Find(TEXT("Claude is thinking..."));
	if (ThinkingIndex != INDEX_NONE)
	{
		// Remove the "thinking" line
		CurrentOutput = CurrentOutput.Left(ThinkingIndex);
		OutputText = FText::FromString(CurrentOutput);
	}

	if (!bSuccess)
	{
		AppendOutput(FString::Printf(TEXT("Error: %s\n\n"), *Response), FLinearColor::Red);
		return;
	}

	// Display Claude's response
	AppendOutput(FString::Printf(TEXT("Claude: %s\n"), *Response), FLinearColor::White);

	// Try to execute any commands in the response
	FString ExecutionLog;
	UWorld* World = GetEditorWorld();

	if (CommandExecutor->ProcessResponse(Response, World, ExecutionLog))
	{
		// Command was executed
		AppendOutput(FString::Printf(TEXT("[Executed]: %s\n\n"), *ExecutionLog), FLinearColor::Green);
	}
	else
	{
		// No command or execution failed
		if (!ExecutionLog.IsEmpty())
		{
			AppendOutput(FString::Printf(TEXT("[Execution Error]: %s\n\n"), *ExecutionLog), FLinearColor::Red);
		}
		else
		{
			// Just a conversational response
			AppendOutput(TEXT("\n"), FLinearColor::White);
		}
	}
}

void STerminalWidget::AppendOutput(const FString& Text, const FLinearColor& Color)
{
	FString CurrentOutput = OutputText.ToString();
	CurrentOutput.Append(Text);
	OutputText = FText::FromString(CurrentOutput);

	if (OutputTextBox.IsValid())
	{
		OutputTextBox->SetText(OutputText);
		// TODO: Scroll to bottom
	}
}

void STerminalWidget::ClearOutput()
{
	OutputText = LOCTEXT("WelcomeMessage", "Claude Terminal v1.0\nType your commands below. Claude will help you manipulate the UE5 world.\n\n");

	if (OutputTextBox.IsValid())
	{
		OutputTextBox->SetText(OutputText);
	}
}

UWorld* STerminalWidget::GetEditorWorld() const
{
	if (GEditor)
	{
		return GEditor->GetEditorWorldContext().World();
	}
	return nullptr;
}

#undef LOCTEXT_NAMESPACE
