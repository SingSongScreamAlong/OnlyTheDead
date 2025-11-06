// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UWorld;
class AActor;

/**
 * Builds context information about the current scene to send to Claude
 * This helps Claude understand what's already in the level
 */
class CLAUDETERMINAL_API FSceneContextBuilder
{
public:
	FSceneContextBuilder();
	~FSceneContextBuilder();

	/**
	 * Build a comprehensive context string describing the current scene
	 * @param World The world to analyze
	 * @param bIncludeSelection Whether to include selected actors
	 * @param MaxActors Maximum number of actors to include
	 * @return Formatted context string for Claude
	 */
	FString BuildContext(UWorld* World, bool bIncludeSelection = true, int32 MaxActors = 100);

	/**
	 * Build context for selected actors only
	 * @return Formatted context string for selected actors
	 */
	FString BuildSelectionContext();

	/**
	 * Get camera location and rotation
	 * @return Formatted camera info string
	 */
	FString GetCameraInfo();

	/**
	 * Get level bounds and statistics
	 * @param World The world to analyze
	 * @return Formatted level info string
	 */
	FString GetLevelInfo(UWorld* World);

private:
	// Helper to format actor info
	FString FormatActorInfo(AActor* Actor, bool bDetailed = false);

	// Helper to get actor type name
	FString GetActorTypeName(AActor* Actor);

	// Helper to format vector
	FString FormatVector(const FVector& Vec);

	// Helper to format rotator
	FString FormatRotator(const FRotator& Rot);
};
