// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

/**
 * Conversation turn - stores one user command and its results
 */
struct FConversationTurn
{
	FString UserCommand;
	FString ClaudeResponse;
	double Timestamp;

	// Actors created/modified in this turn
	TArray<TWeakObjectPtr<AActor>> ReferencedActors;

	// Locations referenced
	TArray<FVector> ReferencedLocations;

	// Keywords for searching history
	TArray<FString> Keywords;
};

/**
 * Anaphora resolution result (resolving "it", "them", "those")
 */
struct FAnaphoraResolution
{
	bool bResolved = false;
	TArray<AActor*> ResolvedActors;
	FVector ResolvedLocation = FVector::ZeroVector;
	FString ResolutionType; // "it", "them", "that location"
};

/**
 * Conversation memory system for context tracking
 * TIER 2.5 Feature
 */
class CLAUDETERMINALEDITOR_API FConversationMemorySystem
{
public:
	FConversationMemorySystem();
	~FConversationMemorySystem();

	/**
	 * Add a new conversation turn
	 * @param UserCommand What the user said
	 * @param ClaudeResponse Claude's response
	 * @param ReferencedActors Actors created or modified
	 * @param ReferencedLocations Locations mentioned
	 */
	void AddTurn(
		const FString& UserCommand,
		const FString& ClaudeResponse,
		const TArray<AActor*>& ReferencedActors,
		const TArray<FVector>& ReferencedLocations);

	/**
	 * Resolve anaphoric reference (it, them, that, etc.)
	 * @param Reference Reference string ("it", "them", "those", "that")
	 * @param CurrentActors Currently selected actors (for context)
	 * @return Resolution result
	 */
	FAnaphoraResolution ResolveReference(const FString& Reference, const TArray<AActor*>& CurrentActors);

	/**
	 * Get recent conversation context for Claude
	 * @param NumTurns Number of recent turns to include
	 * @return Formatted context string
	 */
	FString GetRecentContext(int32 NumTurns = 5) const;

	/**
	 * Search conversation history for commands
	 * @param Query Search query
	 * @return Matching conversation turns
	 */
	TArray<FConversationTurn> SearchHistory(const FString& Query) const;

	/**
	 * Get last referenced actors
	 * @param Count Number of recent turns to check
	 * @return Most recently referenced actors
	 */
	TArray<AActor*> GetLastReferencedActors(int32 Count = 1) const;

	/**
	 * Get last referenced location
	 * @return Most recent location or zero vector
	 */
	FVector GetLastReferencedLocation() const;

	/**
	 * Clear conversation history
	 */
	void ClearHistory();

	/**
	 * Get total number of turns in history
	 */
	int32 GetHistorySize() const { return ConversationHistory.Num(); }

	/**
	 * Check if query contains anaphoric reference
	 * @param Query User query
	 * @return true if contains "it", "them", etc.
	 */
	static bool ContainsAnaphoricReference(const FString& Query);

private:
	// Extract keywords from command
	TArray<FString> ExtractKeywords(const FString& Command) const;

	// Determine what type of reference (singular/plural/location)
	FString DetermineReferenceType(const FString& Reference) const;

	// Conversation history (FIFO, max 100 turns)
	TArray<FConversationTurn> ConversationHistory;

	// Maximum history size
	static const int32 MaxHistorySize = 100;
};
