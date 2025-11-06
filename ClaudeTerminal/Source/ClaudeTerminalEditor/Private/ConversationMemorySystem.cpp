// Copyright Epic Games, Inc. All Rights Reserved.

#include "ConversationMemorySystem.h"
#include "Misc/DateTime.h"

FConversationMemorySystem::FConversationMemorySystem()
{
}

FConversationMemorySystem::~FConversationMemorySystem()
{
}

void FConversationMemorySystem::AddTurn(
	const FString& UserCommand,
	const FString& ClaudeResponse,
	const TArray<AActor*>& ReferencedActors,
	const TArray<FVector>& ReferencedLocations)
{
	FConversationTurn Turn;
	Turn.UserCommand = UserCommand;
	Turn.ClaudeResponse = ClaudeResponse;
	Turn.Timestamp = FPlatformTime::Seconds();
	Turn.ReferencedLocations = ReferencedLocations;

	// Store weak pointers to actors
	for (AActor* Actor : ReferencedActors)
	{
		if (Actor)
		{
			Turn.ReferencedActors.Add(Actor);
		}
	}

	// Extract keywords
	Turn.Keywords = ExtractKeywords(UserCommand);

	// Add to history
	ConversationHistory.Add(Turn);

	// Trim if too large
	if (ConversationHistory.Num() > MaxHistorySize)
	{
		ConversationHistory.RemoveAt(0);
	}

	UE_LOG(LogTemp, Verbose, TEXT("ConversationMemory: Added turn with %d actors, %d locations"),
		ReferencedActors.Num(), ReferencedLocations.Num());
}

FAnaphoraResolution FConversationMemorySystem::ResolveReference(
	const FString& Reference,
	const TArray<AActor*>& CurrentActors)
{
	FAnaphoraResolution Result;

	FString LowerRef = Reference.ToLower();

	// Determine reference type
	Result.ResolutionType = DetermineReferenceType(LowerRef);

	// Resolve based on type
	if (LowerRef.Contains(TEXT("it")) || LowerRef.Contains(TEXT("that")))
	{
		// Singular reference - get last single actor
		TArray<AActor*> LastActors = GetLastReferencedActors(1);
		if (LastActors.Num() > 0 && LastActors[0].IsValid())
		{
			Result.ResolvedActors.Add(LastActors[0]);
			Result.bResolved = true;
		}
		else if (CurrentActors.Num() > 0)
		{
			// Fallback to current selection
			Result.ResolvedActors.Add(CurrentActors[0]);
			Result.bResolved = true;
		}
	}
	else if (LowerRef.Contains(TEXT("them")) || LowerRef.Contains(TEXT("those")) || LowerRef.Contains(TEXT("these")))
	{
		// Plural reference - get all recent actors
		TArray<AActor*> LastActors = GetLastReferencedActors(3);
		for (AActor* Actor : LastActors)
		{
			if (Actor && Actor->IsValidLowLevel())
			{
				Result.ResolvedActors.AddUnique(Actor);
			}
		}

		if (Result.ResolvedActors.Num() > 0)
		{
			Result.bResolved = true;
		}
		else if (CurrentActors.Num() > 0)
		{
			// Fallback to current selection
			Result.ResolvedActors = CurrentActors;
			Result.bResolved = true;
		}
	}
	else if (LowerRef.Contains(TEXT("there")) || LowerRef.Contains(TEXT("that location")))
	{
		// Location reference
		Result.ResolvedLocation = GetLastReferencedLocation();
		Result.bResolved = !Result.ResolvedLocation.IsZero();
	}

	if (Result.bResolved)
	{
		UE_LOG(LogTemp, Log, TEXT("ConversationMemory: Resolved '%s' to %d actors, location: %s"),
			*Reference, Result.ResolvedActors.Num(), *Result.ResolvedLocation.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ConversationMemory: Failed to resolve '%s'"), *Reference);
	}

	return Result;
}

FString FConversationMemorySystem::GetRecentContext(int32 NumTurns) const
{
	FString Context = TEXT("Recent conversation context:\n");

	int32 StartIndex = FMath::Max(0, ConversationHistory.Num() - NumTurns);

	for (int32 i = StartIndex; i < ConversationHistory.Num(); i++)
	{
		const FConversationTurn& Turn = ConversationHistory[i];

		Context += FString::Printf(TEXT("\nTurn %d:\n"), i + 1);
		Context += FString::Printf(TEXT("  User: %s\n"), *Turn.UserCommand);
		Context += FString::Printf(TEXT("  Claude: %s\n"), *Turn.ClaudeResponse.Left(100)); // Truncate long responses

		if (Turn.ReferencedActors.Num() > 0)
		{
			Context += FString::Printf(TEXT("  Actors created/modified: %d\n"), Turn.ReferencedActors.Num());
		}

		if (Turn.ReferencedLocations.Num() > 0)
		{
			Context += FString::Printf(TEXT("  Locations: "));
			for (const FVector& Loc : Turn.ReferencedLocations)
			{
				Context += FString::Printf(TEXT("(%.0f, %.0f, %.0f) "), Loc.X, Loc.Y, Loc.Z);
			}
			Context += TEXT("\n");
		}
	}

	return Context;
}

TArray<FConversationTurn> FConversationMemorySystem::SearchHistory(const FString& Query) const
{
	TArray<FConversationTurn> Results;

	FString LowerQuery = Query.ToLower();

	for (const FConversationTurn& Turn : ConversationHistory)
	{
		// Check if query matches command or keywords
		if (Turn.UserCommand.ToLower().Contains(LowerQuery))
		{
			Results.Add(Turn);
			continue;
		}

		for (const FString& Keyword : Turn.Keywords)
		{
			if (Keyword.Contains(LowerQuery))
			{
				Results.Add(Turn);
				break;
			}
		}
	}

	return Results;
}

TArray<AActor*> FConversationMemorySystem::GetLastReferencedActors(int32 Count) const
{
	TArray<AActor*> Actors;

	// Search backward through history
	for (int32 i = ConversationHistory.Num() - 1; i >= 0 && Actors.Num() < Count; i--)
	{
		const FConversationTurn& Turn = ConversationHistory[i];

		for (const TWeakObjectPtr<AActor>& WeakActor : Turn.ReferencedActors)
		{
			if (WeakActor.IsValid())
			{
				Actors.AddUnique(WeakActor.Get());
				if (Actors.Num() >= Count)
					break;
			}
		}
	}

	return Actors;
}

FVector FConversationMemorySystem::GetLastReferencedLocation() const
{
	// Search backward for most recent location
	for (int32 i = ConversationHistory.Num() - 1; i >= 0; i--)
	{
		const FConversationTurn& Turn = ConversationHistory[i];
		if (Turn.ReferencedLocations.Num() > 0)
		{
			return Turn.ReferencedLocations[0];
		}
	}

	return FVector::ZeroVector;
}

void FConversationMemorySystem::ClearHistory()
{
	ConversationHistory.Empty();
	UE_LOG(LogTemp, Log, TEXT("ConversationMemory: History cleared"));
}

bool FConversationMemorySystem::ContainsAnaphoricReference(const FString& Query)
{
	FString LowerQuery = Query.ToLower();

	// Check for common anaphoric references
	TArray<FString> Anaphoras = {
		TEXT("it"), TEXT("them"), TEXT("those"), TEXT("these"),
		TEXT("that"), TEXT("this"), TEXT("there"), TEXT("here")
	};

	for (const FString& Anaphora : Anaphoras)
	{
		if (LowerQuery.Contains(Anaphora))
		{
			return true;
		}
	}

	return false;
}

TArray<FString> FConversationMemorySystem::ExtractKeywords(const FString& Command) const
{
	TArray<FString> Keywords;

	// Split into words
	TArray<FString> Words;
	Command.ParseIntoArray(Words, TEXT(" "), true);

	// Common stopwords to ignore
	TArray<FString> StopWords = {
		TEXT("a"), TEXT("an"), TEXT("the"), TEXT("at"), TEXT("to"),
		TEXT("in"), TEXT("on"), TEXT("of"), TEXT("and"), TEXT("or"),
		TEXT("but"), TEXT("for"), TEXT("with")
	};

	for (const FString& Word : Words)
	{
		FString CleanWord = Word.ToLower().TrimStartAndEnd();

		// Skip stopwords and very short words
		if (CleanWord.Len() < 3 || StopWords.Contains(CleanWord))
			continue;

		Keywords.AddUnique(CleanWord);
	}

	return Keywords;
}

FString FConversationMemorySystem::DetermineReferenceType(const FString& Reference) const
{
	FString LowerRef = Reference.ToLower();

	if (LowerRef.Contains(TEXT("it")) || LowerRef.Contains(TEXT("that")))
		return TEXT("singular");

	if (LowerRef.Contains(TEXT("them")) || LowerRef.Contains(TEXT("those")) || LowerRef.Contains(TEXT("these")))
		return TEXT("plural");

	if (LowerRef.Contains(TEXT("there")) || LowerRef.Contains(TEXT("here")))
		return TEXT("location");

	return TEXT("unknown");
}
