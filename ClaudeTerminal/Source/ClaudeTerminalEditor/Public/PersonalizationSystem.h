// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

/**
 * User preference category
 */
UENUM()
enum class EPreferenceCategory : uint8
{
	AssetUsage,           // Frequently used assets
	MaterialPreference,   // Preferred materials
	PlacementStyle,       // Spacing, alignment patterns
	CommandPhrasing,      // Language patterns
	WorkflowPattern,      // Common operation sequences
	TimeOfDay,            // Preferred lighting times
	ScalePreference,      // Typical scale ranges
	ColorPalette          // Color preferences
};

/**
 * Usage statistics for an asset
 */
struct FAssetUsageStats
{
	FString AssetPath;
	int32 TimesUsed = 0;
	double LastUsedTime = 0.0;
	double FirstUsedTime = 0.0;
	TArray<FString> UsedInContexts; // What commands/situations
	float AverageScale = 1.0f;
};

/**
 * Command pattern learned from user
 */
struct FCommandPattern
{
	FString PatternDescription;
	TArray<FString> ExamplePhrases;  // User's various ways of saying it
	FString StandardizedCommand;     // What it maps to
	int32 UsageCount = 0;
	float Confidence = 0.0f;         // 0.0-1.0
};

/**
 * Workflow sequence
 */
struct FWorkflowSequence
{
	FString SequenceName;
	TArray<FString> CommandSequence;
	int32 TimesObserved = 0;
	bool bSuggestAsShortcut = false;
};

/**
 * Placement preference data
 */
struct FPlacementPreference
{
	FString ActorType;               // Type of actor
	float PreferredSpacing = 100.0f; // Distance between similar objects
	float PreferredScale = 1.0f;
	FRotator PreferredRotation = FRotator::ZeroRotator;
	bool bAlignToGrid = false;
	bool bRandomRotation = false;
};

/**
 * User profile with learned preferences
 */
struct FUserProfile
{
	FString UserName;
	TMap<FString, FAssetUsageStats> AssetUsageHistory;
	TArray<FCommandPattern> LearnedPatterns;
	TArray<FWorkflowSequence> CommonWorkflows;
	TMap<FString, FPlacementPreference> PlacementPreferences;
	TArray<FString> FavoriteAssets;
	TArray<FLinearColor> PreferredColors;
	double TotalSessionTime = 0.0;
	int32 TotalCommandsExecuted = 0;
	double SessionStartTime = 0.0;
};

/**
 * Recommendation for user
 */
struct FUserRecommendation
{
	FString RecommendationType;      // "asset", "command", "workflow"
	FString Title;
	FString Description;
	float Confidence = 0.0f;         // 0.0-1.0
	TArray<FString> Reasons;         // Why this recommendation
};

/**
 * Learning & Personalization System
 * TIER 4.4 Feature - Revolutionary
 *
 * Learns user preferences and adapts the system to their style:
 * - Tracks asset usage patterns
 * - Learns command phrasing variations
 * - Identifies common workflows
 * - Prioritizes frequently-used assets in search
 * - Suggests shortcuts for repeated sequences
 */
class CLAUDETERMINALEDITOR_API FPersonalizationSystem
{
public:
	/**
	 * Initialize personalization system
	 * @param UserName User identifier
	 * @return true if initialization successful
	 */
	static bool Initialize(const FString& UserName);

	/**
	 * Shutdown and save profile
	 */
	static void Shutdown();

	/**
	 * Record asset usage
	 * @param AssetPath Path to asset used
	 * @param Context Context/command that used it
	 * @param Scale Scale it was placed at
	 */
	static void RecordAssetUsage(const FString& AssetPath, const FString& Context, float Scale = 1.0f);

	/**
	 * Record command usage
	 * @param Command Command executed
	 * @param bSuccessful Whether command succeeded
	 */
	static void RecordCommandUsage(const FString& Command, bool bSuccessful);

	/**
	 * Record placement action
	 * @param ActorType Type of actor placed
	 * @param Scale Scale used
	 * @param Rotation Rotation used
	 * @param NearbyActors Actors near placement
	 */
	static void RecordPlacement(
		const FString& ActorType,
		float Scale,
		const FRotator& Rotation,
		const TArray<AActor*>& NearbyActors);

	/**
	 * Get personalized asset recommendations
	 * @param Context Current context
	 * @param Count Number of recommendations
	 * @return Array of recommended assets
	 */
	static TArray<FString> GetAssetRecommendations(const FString& Context, int32 Count = 5);

	/**
	 * Prioritize search results based on usage
	 * @param SearchResults Original search results
	 * @param Query Search query
	 * @return Re-ranked results
	 */
	static TArray<FString> PrioritizeSearchResults(
		const TArray<FString>& SearchResults,
		const FString& Query);

	/**
	 * Get suggested command completion
	 * @param PartialCommand Incomplete command
	 * @return Array of suggestions
	 */
	static TArray<FString> GetCommandSuggestions(const FString& PartialCommand);

	/**
	 * Learn command pattern
	 * @param UserPhrase How user phrased it
	 * @param ExecutedCommand What command was executed
	 */
	static void LearnCommandPattern(const FString& UserPhrase, const FString& ExecutedCommand);

	/**
	 * Get learned command pattern
	 * @param UserPhrase User's phrasing
	 * @param OutStandardCommand Standardized command
	 * @return true if pattern found
	 */
	static bool GetLearnedPattern(const FString& UserPhrase, FString& OutStandardCommand);

	/**
	 * Detect workflow sequences
	 * @param RecentCommands Recent command history
	 * @return Detected workflow if found
	 */
	static bool DetectWorkflow(const TArray<FString>& RecentCommands, FWorkflowSequence& OutWorkflow);

	/**
	 * Get workflow recommendations
	 * @param CurrentContext Current situation
	 * @return Array of workflow suggestions
	 */
	static TArray<FUserRecommendation> GetWorkflowRecommendations(const FString& CurrentContext);

	/**
	 * Get placement suggestions
	 * @param ActorType Type to place
	 * @return Suggested placement preferences
	 */
	static FPlacementPreference GetPlacementSuggestion(const FString& ActorType);

	/**
	 * Adapt to user's color preferences
	 * @param UsedColor Color user selected
	 */
	static void LearnColorPreference(const FLinearColor& UsedColor);

	/**
	 * Get recommended colors
	 * @param Count Number of colors to suggest
	 * @return Array of recommended colors
	 */
	static TArray<FLinearColor> GetRecommendedColors(int32 Count = 5);

	/**
	 * Get usage statistics
	 * @param OutStats Statistics map
	 * @return Number of tracked assets
	 */
	static int32 GetUsageStatistics(TMap<FString, FAssetUsageStats>& OutStats);

	/**
	 * Get user profile summary
	 * @return Profile summary text
	 */
	static FString GetProfileSummary();

	/**
	 * Save profile to disk
	 * @return true if save successful
	 */
	static bool SaveProfile();

	/**
	 * Load profile from disk
	 * @param UserName User to load
	 * @return true if load successful
	 */
	static bool LoadProfile(const FString& UserName);

	/**
	 * Reset learning data
	 */
	static void ResetProfile();

	/**
	 * Export profile to JSON
	 * @param OutJSON JSON string
	 * @return true if export successful
	 */
	static bool ExportToJSON(FString& OutJSON);

	/**
	 * Import profile from JSON
	 * @param JSON JSON string
	 * @return true if import successful
	 */
	static bool ImportFromJSON(const FString& JSON);

	/**
	 * Is system initialized
	 */
	static bool IsInitialized() { return bIsInitialized; }

	/**
	 * Get current user profile
	 */
	static FUserProfile* GetProfile() { return &CurrentProfile; }

private:
	// Current user profile
	static FUserProfile CurrentProfile;

	// Recent command history for pattern detection
	static TArray<FString> RecentCommands;

	// Initialization state
	static bool bIsInitialized;

	// Calculate asset relevance score
	static float CalculateAssetRelevance(const FAssetUsageStats& Stats, const FString& Context);

	// Calculate command similarity
	static float CalculateCommandSimilarity(const FString& Command1, const FString& Command2);

	// Detect repeating patterns
	static bool DetectRepeatingPattern(const TArray<FString>& Commands, int32& OutPatternLength);

	// Calculate placement consistency
	static float CalculatePlacementConsistency(const FString& ActorType);

	// Update workflow detection
	static void UpdateWorkflowDetection(const FString& NewCommand);

	// Get profile file path
	static FString GetProfileFilePath(const FString& UserName);

	// Serialize profile to string
	static FString SerializeProfile(const FUserProfile& Profile);

	// Deserialize profile from string
	static bool DeserializeProfile(const FString& Data, FUserProfile& OutProfile);
};
