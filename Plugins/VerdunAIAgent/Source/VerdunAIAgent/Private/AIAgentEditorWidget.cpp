// Copyright Only The Dead Development Team. All Rights Reserved.

#include "AIAgentEditorWidget.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBorder.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "AIAgentEditorWidget"

void SAIAgentEditorWidget::Construct(const FArguments& InArgs)
{
	// Initialize agent
	AgentCore = NewObject<UAIAgentCore>();

	// Bind delegates
	AgentCore->OnTaskStarted.AddRaw(this, &SAIAgentEditorWidget::OnAgentTaskStarted);
	AgentCore->OnTaskCompleted.AddRaw(this, &SAIAgentEditorWidget::OnAgentTaskCompleted);
	AgentCore->OnThinking.AddRaw(this, &SAIAgentEditorWidget::OnAgentThinking);
	AgentCore->OnLog.AddRaw(this, &SAIAgentEditorWidget::OnAgentLog);

	// Initialize provider options
	ProviderOptions.Add(MakeShareable(new FString(TEXT("Anthropic (Claude)"))));
	ProviderOptions.Add(MakeShareable(new FString(TEXT("OpenAI (GPT-4)"))));
	ProviderOptions.Add(MakeShareable(new FString(TEXT("Local Model"))));
	SelectedProvider = ProviderOptions[0];

	// Initialize config
	CurrentConfig.Provider = ELLMProvider::Anthropic;
	CurrentConfig.ModelName = TEXT("claude-3-5-sonnet-20241022");
	CurrentProgress = 0.0f;

	// Set default project context for Verdun game
	FString DefaultContext = TEXT(
		"This is a WWI survival game set at the Battle of Verdun (1916).\n"
		"Key terms:\n"
		"- Trench: 2m deep, 1.5m wide defensive position\n"
		"- Dugout: Underground shelter in trench walls\n"
		"- No Man's Land: Area between opposing trenches\n"
		"- Fort Douaumont, Fort Vaux: Major fortifications\n"
		"- Artillery crater: Shell impact site, 5-10m diameter\n"
		"Historical accuracy is crucial."
	);

	// Build UI
	ChildSlot
	[
		SNew(SVerticalBox)

		// Title
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Title", "Verdun AI Agent - Autonomous Development Assistant"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSeparator)
		]

		// Configuration Panel
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10)
		[
			CreateConfigurationPanel()
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSeparator)
		]

		// Command Panel
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10)
		[
			CreateCommandPanel()
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSeparator)
		]

		// Progress Panel
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10)
		[
			CreateProgressPanel()
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSeparator)
		]

		// Log Panel
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(10)
		[
			CreateLogPanel()
		]
	];

	// Set project context
	AgentCore->SetProjectContext(DefaultContext);

	// Register tick
	RegisterActiveTimer(0.1f, FWidgetActiveTimerDelegate::CreateSP(this, &SAIAgentEditorWidget::UpdateUI));

	AddLogEntry(TEXT("AI Agent initialized. Ready for commands."));
}

SAIAgentEditorWidget::~SAIAgentEditorWidget()
{
	if (AgentCore)
	{
		AgentCore->OnTaskStarted.RemoveAll(this);
		AgentCore->OnTaskCompleted.RemoveAll(this);
		AgentCore->OnThinking.RemoveAll(this);
		AgentCore->OnLog.RemoveAll(this);
	}
}

TSharedRef<SWidget> SAIAgentEditorWidget::CreateConfigurationPanel()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ConfigTitle", "LLM Configuration"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]

		// API Key
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 10, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("APIKey", "API Key:"))
				.MinDesiredWidth(100)
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(APIKeyInput, SEditableTextBox)
				.HintText(LOCTEXT("APIKeyHint", "Enter your API key here"))
				.IsPassword(true)
			]
		]

		// Model Name
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 10, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ModelName", "Model:"))
				.MinDesiredWidth(100)
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(ModelNameInput, SEditableTextBox)
				.Text(FText::FromString(TEXT("claude-3-5-sonnet-20241022")))
			]
		]

		// Project Context
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ProjectContext", "Project Context (helps AI understand your project):"))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SAssignNew(ProjectContextInput, SEditableTextBox)
			.Text(FText::FromString(TEXT("WWI Verdun game project")))
		];
}

TSharedRef<SWidget> SAIAgentEditorWidget::CreateCommandPanel()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CommandTitle", "Natural Language Commands"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]

		// Command input
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SAssignNew(CommandInput, SEditableTextBox)
			.HintText(LOCTEXT("CommandHint", "e.g., 'Create a French trench system from coordinates (0,0,0) to (1000,0,0)'"))
		]

		// Buttons
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 5, 0)
			[
				SAssignNew(ExecuteButton, SButton)
				.Text(LOCTEXT("Execute", "Execute Command"))
				.OnClicked(this, &SAIAgentEditorWidget::OnExecuteCommand)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(StopButton, SButton)
				.Text(LOCTEXT("Stop", "Stop"))
				.OnClicked(this, &SAIAgentEditorWidget::OnStopExecution)
				.IsEnabled(false)
			]
		];
}

TSharedRef<SWidget> SAIAgentEditorWidget::CreateProgressPanel()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ProgressTitle", "Agent Status"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SAssignNew(StatusText, STextBlock)
			.Text(LOCTEXT("StatusIdle", "Idle - Waiting for command"))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SAssignNew(ProgressBar, SProgressBar)
			.Percent(0.0f)
		];
}

TSharedRef<SWidget> SAIAgentEditorWidget::CreateLogPanel()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LogTitle", "Agent Activity Log"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearLog", "Clear"))
				.OnClicked(this, &SAIAgentEditorWidget::OnClearLog)
			]
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(5)
		[
			SAssignNew(LogListView, SListView<TSharedPtr<FAgentLogEntry>>)
			.ListItemsSource(&LogEntries)
			.OnGenerateRow(this, &SAIAgentEditorWidget::GenerateLogRow)
		];
}

FReply SAIAgentEditorWidget::OnExecuteCommand()
{
	FString Command = CommandInput->GetText().ToString();
	if (Command.IsEmpty())
	{
		AddLogEntry(TEXT("Please enter a command"), true);
		return FReply::Handled();
	}

	// Update config from UI
	CurrentConfig.APIKey = APIKeyInput->GetText().ToString();
	CurrentConfig.ModelName = ModelNameInput->GetText().ToString();

	if (CurrentConfig.APIKey.IsEmpty())
	{
		AddLogEntry(TEXT("Please enter your API key in the configuration panel"), true);
		return FReply::Handled();
	}

	// Initialize and execute
	AgentCore->Initialize(CurrentConfig);
	AgentCore->ExecuteCommand(Command);

	ExecuteButton->SetEnabled(false);
	StopButton->SetEnabled(true);

	return FReply::Handled();
}

FReply SAIAgentEditorWidget::OnStopExecution()
{
	AgentCore->StopExecution();

	ExecuteButton->SetEnabled(true);
	StopButton->SetEnabled(false);

	return FReply::Handled();
}

FReply SAIAgentEditorWidget::OnClearLog()
{
	LogEntries.Empty();
	LogListView->RequestListRefresh();
	return FReply::Handled();
}

void SAIAgentEditorWidget::OnProviderChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	SelectedProvider = NewSelection;
	// Update provider in config
}

void SAIAgentEditorWidget::OnAgentTaskStarted(const FString& TaskDescription)
{
	AddLogEntry(FString::Printf(TEXT("Task started: %s"), *TaskDescription));
}

void SAIAgentEditorWidget::OnAgentTaskCompleted(const FString& TaskDescription, bool bSuccess)
{
	if (bSuccess)
	{
		AddLogEntry(FString::Printf(TEXT("Task completed successfully: %s"), *TaskDescription));
	}
	else
	{
		AddLogEntry(FString::Printf(TEXT("Task failed: %s"), *TaskDescription), true);
	}

	ExecuteButton->SetEnabled(true);
	StopButton->SetEnabled(false);
	CurrentProgress = 0.0f;
}

void SAIAgentEditorWidget::OnAgentThinking(const FString& ThoughtProcess, float Progress)
{
	CurrentThought = ThoughtProcess;
	CurrentProgress = Progress;
	AddLogEntry(ThoughtProcess);
}

void SAIAgentEditorWidget::OnAgentLog(const FString& LogMessage)
{
	AddLogEntry(LogMessage);
}

void SAIAgentEditorWidget::AddLogEntry(const FString& Message, bool bIsError)
{
	LogEntries.Add(MakeShareable(new FAgentLogEntry(Message, bIsError)));
	LogListView->RequestListRefresh();
	LogListView->RequestScrollIntoView(LogEntries.Last());
}

TSharedRef<ITableRow> SAIAgentEditorWidget::GenerateLogRow(TSharedPtr<FAgentLogEntry> Entry, const TSharedRef<STableViewBase>& OwnerTable)
{
	FString TimeStr = Entry->Timestamp.ToString(TEXT("%H:%M:%S"));
	FLinearColor TextColor = Entry->bIsError ? FLinearColor::Red : FLinearColor::White;

	return SNew(STableRow<TSharedPtr<FAgentLogEntry>>, OwnerTable)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5, 2)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TimeStr))
				.ColorAndOpacity(FLinearColor::Gray)
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(5, 2)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Entry->Message))
				.ColorAndOpacity(TextColor)
				.AutoWrapText(true)
			]
		];
}

EActiveTimerReturnType SAIAgentEditorWidget::UpdateUI(double InCurrentTime, float InDeltaTime)
{
	// Update status text
	StatusText->SetText(GetStatusText());
	StatusText->SetColorAndOpacity(GetStatusColor());

	// Update progress bar
	ProgressBar->SetPercent(CurrentProgress);

	return EActiveTimerReturnType::Continue;
}

FText SAIAgentEditorWidget::GetStatusText() const
{
	if (!AgentCore)
	{
		return LOCTEXT("StatusUninitialized", "Uninitialized");
	}

	EAgentTaskStatus Status = AgentCore->GetCurrentStatus();

	switch (Status)
	{
	case EAgentTaskStatus::Idle:
		return LOCTEXT("StatusIdle", "Idle - Waiting for command");
	case EAgentTaskStatus::Planning:
		return LOCTEXT("StatusPlanning", "Planning - Analyzing request...");
	case EAgentTaskStatus::Executing:
		return FText::FromString(CurrentThought.IsEmpty() ? TEXT("Executing...") : CurrentThought);
	case EAgentTaskStatus::Verifying:
		return LOCTEXT("StatusVerifying", "Verifying completion...");
	case EAgentTaskStatus::Completed:
		return LOCTEXT("StatusCompleted", "Completed");
	case EAgentTaskStatus::Failed:
		return LOCTEXT("StatusFailed", "Failed");
	default:
		return LOCTEXT("StatusUnknown", "Unknown");
	}
}

FSlateColor SAIAgentEditorWidget::GetStatusColor() const
{
	if (!AgentCore)
	{
		return FLinearColor::Gray;
	}

	EAgentTaskStatus Status = AgentCore->GetCurrentStatus();

	switch (Status)
	{
	case EAgentTaskStatus::Idle:
		return FLinearColor::Gray;
	case EAgentTaskStatus::Planning:
	case EAgentTaskStatus::Executing:
	case EAgentTaskStatus::Verifying:
		return FLinearColor::Yellow;
	case EAgentTaskStatus::Completed:
		return FLinearColor::Green;
	case EAgentTaskStatus::Failed:
		return FLinearColor::Red;
	default:
		return FLinearColor::Gray;
	}
}

#undef LOCTEXT_NAMESPACE
