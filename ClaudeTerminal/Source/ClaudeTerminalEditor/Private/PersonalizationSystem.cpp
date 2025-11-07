// Copyright Epic Games, Inc. All Rights Reserved.

#include "PersonalizationSystem.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformTime.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"

// Static member initialization
FUserProfile FPersonalizationSystem::CurrentProfile;
TArray<FString> FPersonalizationSystem::RecentCommands;
bool FPersonalizationSystem::bIsInitialized = false;

bool FPersonalizationSystem::Initialize(const FString& UserName)
{
	UE_LOG(LogTemp, Log, TEXT("Personalization: Initializing for user '%s'"), *UserName);

	CurrentProfile.UserName = UserName;
	CurrentProfile.SessionStartTime = FPlatformTime::Seconds();

	// Try to load existing profile
	if (LoadProfile(UserName))
	{
		UE_LOG(LogTemp, Log, TEXT("Personalization: Loaded existing profile with %d assets tracked"),
			CurrentProfile.AssetUsageHistory.Num());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Personalization: Creating new profile for %s"), *UserName);
	}

	bIsInitialized = true;
	return true;
}

void FPersonalizationSystem::Shutdown()
{
	if (!bIsInitialized)
		return;

	UE_LOG(LogTemp, Log, TEXT("Personalization: Shutting down, saving profile"));

	// Update session time
	CurrentProfile.TotalSessionTime += FPlatformTime::Seconds() - CurrentProfile.SessionStartTime;

	// Save profile
	SaveProfile();

	bIsInitialized = false;
}

void FPersonalizationSystem::RecordAssetUsage(const FString& AssetPath, const FString& Context, float Scale)
{
	if (!bIsInitialized)
		return;

	FAssetUsageStats* Stats = CurrentProfile.AssetUsageHistory.Find(AssetPath);

	if (!Stats)
	{
		// New asset
		FAssetUsageStats NewStats;
		NewStats.AssetPath = AssetPath;
		NewStats.FirstUsedTime = FPlatformTime::Seconds();
		NewStats.LastUsedTime = NewStats.FirstUsedTime;
		NewStats.TimesUsed = 1;
		NewStats.AverageScale = Scale;
		NewStats.UsedInContexts.Add(Context);

		CurrentProfile.AssetUsageHistory.Add(AssetPath, NewStats);

		UE_LOG(LogTemp, Log, TEXT("Personalization: First use of asset '%s'"), *AssetPath);
	}
	else
	{
		// Update existing stats
		Stats->TimesUsed++;
		Stats->LastUsedTime = FPlatformTime::Seconds();

		// Update average scale
		Stats->AverageScale = (Stats->AverageScale * (Stats->TimesUsed - 1) + Scale) / Stats->TimesUsed;

		// Track context if new
		if (!Stats->UsedInContexts.Contains(Context))
		{
			Stats->UsedInContexts.Add(Context);
		}

		UE_LOG(LogTemp, Log, TEXT("Personalization: Asset '%s' used %d times"), *AssetPath, Stats->TimesUsed);

		// Add to favorites if used frequently
		if (Stats->TimesUsed >= 10 && !CurrentProfile.FavoriteAssets.Contains(AssetPath))
		{
			CurrentProfile.FavoriteAssets.Add(AssetPath);
			UE_LOG(LogTemp, Log, TEXT("Personalization: Added '%s' to favorites"), *AssetPath);
		}
	}
}

void FPersonalizationSystem::RecordCommandUsage(const FString& Command, bool bSuccessful)
{
	if (!bIsInitialized)
		return;

	CurrentProfile.TotalCommandsExecuted++;

	// Add to recent history
	RecentCommands.Add(Command);
	if (RecentCommands.Num() > 50)
	{
		RecentCommands.RemoveAt(0);
	}

	// Update workflow detection
	UpdateWorkflowDetection(Command);

	UE_LOG(LogTemp, Log, TEXT("Personalization: Command recorded '%s' (success=%d)"), *Command, bSuccessful);
}

void FPersonalizationSystem::RecordPlacement(
	const FString& ActorType,
	float Scale,
	const FRotator& Rotation,
	const TArray<AActor*>& NearbyActors)
{
	if (!bIsInitialized)
		return;

	FPlacementPreference* Pref = CurrentProfile.PlacementPreferences.Find(ActorType);

	if (!Pref)
	{
		// New placement preference
		FPlacementPreference NewPref;
		NewPref.ActorType = ActorType;
		NewPref.PreferredScale = Scale;
		NewPref.PreferredRotation = Rotation;

		// Calculate spacing from nearby actors
		if (NearbyActors.Num() > 0)
		{
			// Would calculate average distance to nearby similar actors
			NewPref.PreferredSpacing = 200.0f; // Placeholder
		}

		CurrentProfile.PlacementPreferences.Add(ActorType, NewPref);
	}
	else
	{
		// Update existing preference (rolling average)
		Pref->PreferredScale = (Pref->PreferredScale + Scale) / 2.0f;

		// Update rotation preference if consistent
		// (simplified - would do proper rotation averaging)
		if (FMath::Abs(Rotation.Yaw - Pref->PreferredRotation.Yaw) < 10.0f)
		{
			Pref->PreferredRotation = Rotation;
		}
		else
		{
			Pref->bRandomRotation = true; // User varies rotation
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Personalization: Recorded placement of '%s' at scale %.2f"), *ActorType, Scale);
}

TArray<FString> FPersonalizationSystem::GetAssetRecommendations(const FString& Context, int32 Count)
{
	TArray<FString> Recommendations;

	if (!bIsInitialized)
		return Recommendations;

	// Score all assets by relevance
	TArray<TPair<FString, float>> ScoredAssets;

	for (const auto& Pair : CurrentProfile.AssetUsageHistory)
	{
		float Score = CalculateAssetRelevance(Pair.Value, Context);
		ScoredAssets.Add(TPair<FString, float>(Pair.Key, Score));
	}

	// Sort by score
	ScoredAssets.Sort([](const TPair<FString, float>& A, const TPair<FString, float>& B) {
		return A.Value > B.Value;
	});

	// Take top N
	for (int32 i = 0; i < FMath::Min(Count, ScoredAssets.Num()); i++)
	{
		Recommendations.Add(ScoredAssets[i].Key);
	}

	UE_LOG(LogTemp, Log, TEXT("Personalization: Generated %d asset recommendations"), Recommendations.Num());

	return Recommendations;
}

TArray<FString> FPersonalizationSystem::PrioritizeSearchResults(
	const TArray<FString>& SearchResults,
	const FString& Query)
{
	if (!bIsInitialized)
		return SearchResults;

	TArray<FString> Prioritized = SearchResults;

	// Re-rank based on usage frequency
	Prioritized.Sort([this](const FString& A, const FString& B) {
		const FAssetUsageStats* StatsA = CurrentProfile.AssetUsageHistory.Find(A);
		const FAssetUsageStats* StatsB = CurrentProfile.AssetUsageHistory.Find(B);

		int32 UsageA = StatsA ? StatsA->TimesUsed : 0;
		int32 UsageB = StatsB ? StatsB->TimesUsed : 0;

		// Favorites get extra boost
		if (CurrentProfile.FavoriteAssets.Contains(A))
			UsageA += 100;
		if (CurrentProfile.FavoriteAssets.Contains(B))
			UsageB += 100;

		return UsageA > UsageB;
	});

	return Prioritized;
}

TArray<FString> FPersonalizationSystem::GetCommandSuggestions(const FString& PartialCommand)
{
	TArray<FString> Suggestions;

	if (!bIsInitialized || PartialCommand.IsEmpty())
		return Suggestions;

	// Check learned patterns
	for (const FCommandPattern& Pattern : CurrentProfile.LearnedPatterns)
	{
		for (const FString& Example : Pattern.ExamplePhrases)
		{
			if (Example.StartsWith(PartialCommand, ESearchCase::IgnoreCase))
			{
				Suggestions.Add(Example);
			}
		}
	}

	// Check recent commands
	for (const FString& RecentCmd : RecentCommands)
	{
		if (RecentCmd.StartsWith(PartialCommand, ESearchCase::IgnoreCase))
		{
			if (!Suggestions.Contains(RecentCmd))
			{
				Suggestions.Add(RecentCmd);
			}
		}
	}

	// Sort by frequency
	Suggestions.Sort([this](const FString& A, const FString& B) {
		int32 CountA = RecentCommands.FilterByPredicate([&A](const FString& Cmd) {
			return Cmd == A;
		}).Num();

		int32 CountB = RecentCommands.FilterByPredicate([&B](const FString& Cmd) {
			return Cmd == B;
		}).Num();

		return CountA > CountB;
	});

	return Suggestions;
}

void FPersonalizationSystem::LearnCommandPattern(const FString& UserPhrase, const FString& ExecutedCommand)
{
	if (!bIsInitialized)
		return;

	// Find existing pattern
	FCommandPattern* ExistingPattern = CurrentProfile.LearnedPatterns.FindByPredicate(
		[&ExecutedCommand](const FCommandPattern& Pattern) {
			return Pattern.StandardizedCommand == ExecutedCommand;
		});

	if (ExistingPattern)
	{
		// Add phrase variant if new
		if (!ExistingPattern->ExamplePhrases.Contains(UserPhrase))
		{
			ExistingPattern->ExamplePhrases.Add(UserPhrase);
			ExistingPattern->UsageCount++;
			ExistingPattern->Confidence = FMath::Min(1.0f, ExistingPattern->Confidence + 0.1f);

			UE_LOG(LogTemp, Log, TEXT("Personalization: Learned new phrase variant '%s' -> '%s'"),
				*UserPhrase, *ExecutedCommand);
		}
	}
	else
	{
		// Create new pattern
		FCommandPattern NewPattern;
		NewPattern.StandardizedCommand = ExecutedCommand;
		NewPattern.ExamplePhrases.Add(UserPhrase);
		NewPattern.UsageCount = 1;
		NewPattern.Confidence = 0.3f;
		NewPattern.PatternDescription = FString::Printf(TEXT("User says '%s' to mean '%s'"),
			*UserPhrase, *ExecutedCommand);

		CurrentProfile.LearnedPatterns.Add(NewPattern);

		UE_LOG(LogTemp, Log, TEXT("Personalization: Created new command pattern"));
	}
}

bool FPersonalizationSystem::GetLearnedPattern(const FString& UserPhrase, FString& OutStandardCommand)
{
	if (!bIsInitialized)
		return false;

	// Find best matching pattern
	float BestScore = 0.0f;
	FCommandPattern* BestPattern = nullptr;

	for (FCommandPattern& Pattern : CurrentProfile.LearnedPatterns)
	{
		for (const FString& Example : Pattern.ExamplePhrases)
		{
			float Similarity = CalculateCommandSimilarity(UserPhrase, Example);

			if (Similarity > BestScore && Similarity > 0.7f)
			{
				BestScore = Similarity;
				BestPattern = &Pattern;
			}
		}
	}

	if (BestPattern)
	{
		OutStandardCommand = BestPattern->StandardizedCommand;
		return true;
	}

	return false;
}

bool FPersonalizationSystem::DetectWorkflow(const TArray<FString>& RecentCommands, FWorkflowSequence& OutWorkflow)
{
	if (RecentCommands.Num() < 3)
		return false;

	// Detect repeating patterns
	int32 PatternLength = 0;
	if (DetectRepeatingPattern(RecentCommands, PatternLength))
	{
		// Extract the pattern
		OutWorkflow.CommandSequence.Empty();
		int32 StartIndex = RecentCommands.Num() - PatternLength;

		for (int32 i = StartIndex; i < RecentCommands.Num(); i++)
		{
			OutWorkflow.CommandSequence.Add(RecentCommands[i]);
		}

		OutWorkflow.SequenceName = TEXT("Detected Pattern");
		OutWorkflow.TimesObserved = 1;

		return true;
	}

	return false;
}

TArray<FUserRecommendation> FPersonalizationSystem::GetWorkflowRecommendations(const FString& CurrentContext)
{
	TArray<FUserRecommendation> Recommendations;

	if (!bIsInitialized)
		return Recommendations;

	// Check for workflows that match current context
	for (const FWorkflowSequence& Workflow : CurrentProfile.CommonWorkflows)
	{
		if (Workflow.bSuggestAsShortcut && Workflow.TimesObserved >= 3)
		{
			FUserRecommendation Rec;
			Rec.RecommendationType = TEXT("workflow");
			Rec.Title = FString::Printf(TEXT("Workflow: %s"), *Workflow.SequenceName);
			Rec.Description = FString::Printf(TEXT("You've done this %d times. Create a shortcut?"),
				Workflow.TimesObserved);
			Rec.Confidence = FMath::Min(1.0f, Workflow.TimesObserved / 10.0f);
			Rec.Reasons.Add(FString::Printf(TEXT("Observed %d times"), Workflow.TimesObserved));

			Recommendations.Add(Rec);
		}
	}

	return Recommendations;
}

FPlacementPreference FPersonalizationSystem::GetPlacementSuggestion(const FString& ActorType)
{
	FPlacementPreference DefaultPref;
	DefaultPref.ActorType = ActorType;
	DefaultPref.PreferredSpacing = 100.0f;
	DefaultPref.PreferredScale = 1.0f;

	if (!bIsInitialized)
		return DefaultPref;

	const FPlacementPreference* Pref = CurrentProfile.PlacementPreferences.Find(ActorType);

	if (Pref)
	{
		UE_LOG(LogTemp, Log, TEXT("Personalization: Suggesting scale %.2f for '%s' based on history"),
			Pref->PreferredScale, *ActorType);
		return *Pref;
	}

	return DefaultPref;
}

void FPersonalizationSystem::LearnColorPreference(const FLinearColor& UsedColor)
{
	if (!bIsInitialized)
		return;

	// Add to color palette if not similar to existing
	bool bSimilarExists = false;

	for (const FLinearColor& ExistingColor : CurrentProfile.PreferredColors)
	{
		float Distance = FVector(UsedColor.R - ExistingColor.R,
			UsedColor.G - ExistingColor.G,
			UsedColor.B - ExistingColor.B).Size();

		if (Distance < 0.2f)
		{
			bSimilarExists = true;
			break;
		}
	}

	if (!bSimilarExists)
	{
		CurrentProfile.PreferredColors.Add(UsedColor);
		UE_LOG(LogTemp, Log, TEXT("Personalization: Learned color preference (R:%.2f G:%.2f B:%.2f)"),
			UsedColor.R, UsedColor.G, UsedColor.B);
	}
}

TArray<FLinearColor> FPersonalizationSystem::GetRecommendedColors(int32 Count)
{
	TArray<FLinearColor> Colors;

	if (!bIsInitialized)
		return Colors;

	// Return most recent colors
	int32 NumToReturn = FMath::Min(Count, CurrentProfile.PreferredColors.Num());

	for (int32 i = CurrentProfile.PreferredColors.Num() - NumToReturn; i < CurrentProfile.PreferredColors.Num(); i++)
	{
		Colors.Add(CurrentProfile.PreferredColors[i]);
	}

	return Colors;
}

int32 FPersonalizationSystem::GetUsageStatistics(TMap<FString, FAssetUsageStats>& OutStats)
{
	if (!bIsInitialized)
		return 0;

	OutStats = CurrentProfile.AssetUsageHistory;
	return OutStats.Num();
}

FString FPersonalizationSystem::GetProfileSummary()
{
	if (!bIsInitialized)
		return TEXT("Personalization system not initialized");

	FString Summary;
	Summary += FString::Printf(TEXT("User: %s\n"), *CurrentProfile.UserName);
	Summary += FString::Printf(TEXT("Total Commands: %d\n"), CurrentProfile.TotalCommandsExecuted);
	Summary += FString::Printf(TEXT("Assets Tracked: %d\n"), CurrentProfile.AssetUsageHistory.Num());
	Summary += FString::Printf(TEXT("Favorite Assets: %d\n"), CurrentProfile.FavoriteAssets.Num());
	Summary += FString::Printf(TEXT("Learned Patterns: %d\n"), CurrentProfile.LearnedPatterns.Num());
	Summary += FString::Printf(TEXT("Common Workflows: %d\n"), CurrentProfile.CommonWorkflows.Num());
	Summary += FString::Printf(TEXT("Session Time: %.1f hours\n"), CurrentProfile.TotalSessionTime / 3600.0);

	// Top 5 assets
	TArray<TPair<FString, int32>> TopAssets;
	for (const auto& Pair : CurrentProfile.AssetUsageHistory)
	{
		TopAssets.Add(TPair<FString, int32>(Pair.Key, Pair.Value.TimesUsed));
	}
	TopAssets.Sort([](const TPair<FString, int32>& A, const TPair<FString, int32>& B) {
		return A.Value > B.Value;
	});

	Summary += TEXT("\nTop Assets:\n");
	for (int32 i = 0; i < FMath::Min(5, TopAssets.Num()); i++)
	{
		Summary += FString::Printf(TEXT("  %d. %s (%d uses)\n"), i + 1, *TopAssets[i].Key, TopAssets[i].Value);
	}

	return Summary;
}

bool FPersonalizationSystem::SaveProfile()
{
	if (!bIsInitialized)
		return false;

	FString FilePath = GetProfileFilePath(CurrentProfile.UserName);
	FString JSON;

	if (ExportToJSON(JSON))
	{
		if (FFileHelper::SaveStringToFile(JSON, *FilePath))
		{
			UE_LOG(LogTemp, Log, TEXT("Personalization: Profile saved to %s"), *FilePath);
			return true;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("Personalization: Failed to save profile"));
	return false;
}

bool FPersonalizationSystem::LoadProfile(const FString& UserName)
{
	FString FilePath = GetProfileFilePath(UserName);

	if (!FPaths::FileExists(FilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("Personalization: No existing profile found at %s"), *FilePath);
		return false;
	}

	FString JSON;
	if (FFileHelper::LoadFileToString(JSON, *FilePath))
	{
		if (ImportFromJSON(JSON))
		{
			UE_LOG(LogTemp, Log, TEXT("Personalization: Profile loaded from %s"), *FilePath);
			return true;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("Personalization: Failed to load profile"));
	return false;
}

void FPersonalizationSystem::ResetProfile()
{
	if (!bIsInitialized)
		return;

	FString UserName = CurrentProfile.UserName;

	CurrentProfile = FUserProfile();
	CurrentProfile.UserName = UserName;
	CurrentProfile.SessionStartTime = FPlatformTime::Seconds();

	RecentCommands.Empty();

	UE_LOG(LogTemp, Log, TEXT("Personalization: Profile reset for %s"), *UserName);
}

bool FPersonalizationSystem::ExportToJSON(FString& OutJSON)
{
	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);

	RootObject->SetStringField(TEXT("UserName"), CurrentProfile.UserName);
	RootObject->SetNumberField(TEXT("TotalCommands"), CurrentProfile.TotalCommandsExecuted);
	RootObject->SetNumberField(TEXT("TotalSessionTime"), CurrentProfile.TotalSessionTime);

	// Asset usage
	TArray<TSharedPtr<FJsonValue>> AssetArray;
	for (const auto& Pair : CurrentProfile.AssetUsageHistory)
	{
		TSharedPtr<FJsonObject> AssetObj = MakeShareable(new FJsonObject);
		AssetObj->SetStringField(TEXT("Path"), Pair.Value.AssetPath);
		AssetObj->SetNumberField(TEXT("TimesUsed"), Pair.Value.TimesUsed);
		AssetObj->SetNumberField(TEXT("AverageScale"), Pair.Value.AverageScale);

		AssetArray.Add(MakeShareable(new FJsonValueObject(AssetObj)));
	}
	RootObject->SetArrayField(TEXT("AssetUsage"), AssetArray);

	// Favorite assets
	TArray<TSharedPtr<FJsonValue>> FavoritesArray;
	for (const FString& Favorite : CurrentProfile.FavoriteAssets)
	{
		FavoritesArray.Add(MakeShareable(new FJsonValueString(Favorite)));
	}
	RootObject->SetArrayField(TEXT("Favorites"), FavoritesArray);

	// Convert to string
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJSON);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	return true;
}

bool FPersonalizationSystem::ImportFromJSON(const FString& JSON)
{
	TSharedPtr<FJsonObject> RootObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JSON);

	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Personalization: Failed to parse JSON"));
		return false;
	}

	CurrentProfile.UserName = RootObject->GetStringField(TEXT("UserName"));
	CurrentProfile.TotalCommandsExecuted = RootObject->GetIntegerField(TEXT("TotalCommands"));
	CurrentProfile.TotalSessionTime = RootObject->GetNumberField(TEXT("TotalSessionTime"));

	// Load asset usage
	const TArray<TSharedPtr<FJsonValue>>* AssetArray;
	if (RootObject->TryGetArrayField(TEXT("AssetUsage"), AssetArray))
	{
		for (const TSharedPtr<FJsonValue>& Value : *AssetArray)
		{
			TSharedPtr<FJsonObject> AssetObj = Value->AsJsonObject();

			FAssetUsageStats Stats;
			Stats.AssetPath = AssetObj->GetStringField(TEXT("Path"));
			Stats.TimesUsed = AssetObj->GetIntegerField(TEXT("TimesUsed"));
			Stats.AverageScale = AssetObj->GetNumberField(TEXT("AverageScale"));

			CurrentProfile.AssetUsageHistory.Add(Stats.AssetPath, Stats);
		}
	}

	// Load favorites
	const TArray<TSharedPtr<FJsonValue>>* FavoritesArray;
	if (RootObject->TryGetArrayField(TEXT("Favorites"), FavoritesArray))
	{
		for (const TSharedPtr<FJsonValue>& Value : *FavoritesArray)
		{
			CurrentProfile.FavoriteAssets.Add(Value->AsString());
		}
	}

	return true;
}

float FPersonalizationSystem::CalculateAssetRelevance(const FAssetUsageStats& Stats, const FString& Context)
{
	float Score = 0.0f;

	// Usage frequency (0-50 points)
	Score += FMath::Min(50.0f, Stats.TimesUsed * 5.0f);

	// Recency (0-30 points)
	double TimeSinceUse = FPlatformTime::Seconds() - Stats.LastUsedTime;
	double DaysSinceUse = TimeSinceUse / 86400.0;
	Score += FMath::Max(0.0f, 30.0f - DaysSinceUse);

	// Context match (0-20 points)
	if (Stats.UsedInContexts.Contains(Context))
	{
		Score += 20.0f;
	}

	return Score;
}

float FPersonalizationSystem::CalculateCommandSimilarity(const FString& Command1, const FString& Command2)
{
	// Simple Levenshtein-style similarity
	// Full implementation would use proper string similarity algorithm

	if (Command1 == Command2)
		return 1.0f;

	int32 CommonWords = 0;
	int32 TotalWords = 0;

	TArray<FString> Words1, Words2;
	Command1.ParseIntoArray(Words1, TEXT(" "));
	Command2.ParseIntoArray(Words2, TEXT(" "));

	TotalWords = FMath::Max(Words1.Num(), Words2.Num());

	for (const FString& Word1 : Words1)
	{
		if (Words2.Contains(Word1))
		{
			CommonWords++;
		}
	}

	return TotalWords > 0 ? (float)CommonWords / TotalWords : 0.0f;
}

bool FPersonalizationSystem::DetectRepeatingPattern(const TArray<FString>& Commands, int32& OutPatternLength)
{
	// Detect if last N commands repeat an earlier pattern

	for (int32 PatternLen = 3; PatternLen <= Commands.Num() / 2; PatternLen++)
	{
		bool bMatches = true;

		for (int32 i = 0; i < PatternLen; i++)
		{
			int32 CurrentIndex = Commands.Num() - PatternLen + i;
			int32 PreviousIndex = Commands.Num() - 2 * PatternLen + i;

			if (PreviousIndex < 0 || Commands[CurrentIndex] != Commands[PreviousIndex])
			{
				bMatches = false;
				break;
			}
		}

		if (bMatches)
		{
			OutPatternLength = PatternLen;
			return true;
		}
	}

	return false;
}

float FPersonalizationSystem::CalculatePlacementConsistency(const FString& ActorType)
{
	const FPlacementPreference* Pref = CurrentProfile.PlacementPreferences.Find(ActorType);

	if (!Pref)
		return 0.0f;

	// Calculate consistency score based on variation
	// Less variation = higher consistency

	float Consistency = 0.5f; // Base consistency

	if (!Pref->bRandomRotation)
		Consistency += 0.25f;

	if (!Pref->bAlignToGrid)
		Consistency += 0.25f;

	return Consistency;
}

void FPersonalizationSystem::UpdateWorkflowDetection(const FString& NewCommand)
{
	// Check if recent commands form a workflow

	if (RecentCommands.Num() < 3)
		return;

	FWorkflowSequence DetectedWorkflow;
	if (DetectWorkflow(RecentCommands, DetectedWorkflow))
	{
		// Check if this workflow already exists
		FWorkflowSequence* Existing = CurrentProfile.CommonWorkflows.FindByPredicate(
			[&DetectedWorkflow](const FWorkflowSequence& WF) {
				return WF.CommandSequence == DetectedWorkflow.CommandSequence;
			});

		if (Existing)
		{
			Existing->TimesObserved++;

			if (Existing->TimesObserved >= 3)
			{
				Existing->bSuggestAsShortcut = true;
				UE_LOG(LogTemp, Log, TEXT("Personalization: Workflow repeated %d times, suggesting shortcut"),
					Existing->TimesObserved);
			}
		}
		else
		{
			CurrentProfile.CommonWorkflows.Add(DetectedWorkflow);
		}
	}
}

FString FPersonalizationSystem::GetProfileFilePath(const FString& UserName)
{
	FString SaveDir = FPaths::ProjectSavedDir() / TEXT("ClaudeTerminal") / TEXT("Profiles");
	FString FileName = UserName + TEXT(".json");

	return SaveDir / FileName;
}
