// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * TIER 1.2: Enhanced Error Message System
 * Provides descriptive, helpful error messages with suggestions and examples
 */
class CLAUDETERMINALEDITOR_API FErrorMessageSystem
{
public:
	/**
	 * Error types for categorization
	 */
	enum class EErrorType
	{
		InvalidParameters,
		NoSelection,
		AssetNotFound,
		InvalidAssetType,
		OperationFailed,
		UnsupportedOperation,
		PermissionDenied
	};

	/**
	 * Generate enhanced error message with suggestions
	 */
	static FString GetEnhancedError(
		EErrorType ErrorType,
		const FString& Context,
		const TArray<FString>& Suggestions = {},
		const TArray<FString>& Examples = {}
	);

	/**
	 * Specific error builders for common scenarios
	 */
	static FString NoActorsSelected(const FString& OperationName);
	static FString AssetNotFound(const FString& AssetQuery, const TArray<FString>& SimilarAssets = {});
	static FString InvalidParameters(const FString& CommandName, const TArray<FString>& RequiredParams);
	static FString OperationFailed(const FString& OperationName, const FString& Reason, const TArray<FString>& Solutions = {});
	static FString UnsupportedAssetType(const FString& AssetType, const TArray<FString>& SupportedTypes);

	/**
	 * Get related commands for suggestions
	 */
	static TArray<FString> GetRelatedCommands(const FString& CommandName);

	/**
	 * Format suggestions section
	 */
	static FString FormatSuggestions(const TArray<FString>& Suggestions);

	/**
	 * Format examples section
	 */
	static FString FormatExamples(const TArray<FString>& Examples);

private:
	/**
	 * Command relationship map for suggestions
	 */
	static TMap<FString, TArray<FString>> CommandRelationships;
	static void InitializeCommandRelationships();
};
