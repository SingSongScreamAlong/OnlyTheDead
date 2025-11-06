// Copyright Epic Games, Inc. All Rights Reserved.

#include "ErrorMessageSystem.h"

TMap<FString, TArray<FString>> FErrorMessageSystem::CommandRelationships;

void FErrorMessageSystem::InitializeCommandRelationships()
{
	if (CommandRelationships.Num() > 0)
	{
		return; // Already initialized
	}

	// Build command relationship map
	CommandRelationships.Add(TEXT("spawn_object"), {TEXT("place_building"), TEXT("place_props"), TEXT("place_vegetation")});
	CommandRelationships.Add(TEXT("place_building"), {TEXT("spawn_object"), TEXT("apply_material"), TEXT("modify_actor")});
	CommandRelationships.Add(TEXT("place_props"), {TEXT("spawn_object"), TEXT("arrange_pattern"), TEXT("duplicate_actor")});
	CommandRelationships.Add(TEXT("apply_material"), {TEXT("set_material_parameter"), TEXT("place_building")});
	CommandRelationships.Add(TEXT("modify_actor"), {TEXT("duplicate_actor"), TEXT("arrange_pattern"), TEXT("delete_actor")});
	CommandRelationships.Add(TEXT("create_landscape"), {TEXT("import_terrain"), TEXT("modify_terrain")});
	CommandRelationships.Add(TEXT("import_terrain"), {TEXT("create_crater"), TEXT("create_trench"), TEXT("modify_terrain")});
	CommandRelationships.Add(TEXT("create_light"), {TEXT("modify_light"), TEXT("set_time_of_day")});
	CommandRelationships.Add(TEXT("set_time_of_day"), {TEXT("create_light"), TEXT("set_weather"), TEXT("set_fog")});
}

FString FErrorMessageSystem::GetEnhancedError(
	EErrorType ErrorType,
	const FString& Context,
	const TArray<FString>& Suggestions,
	const TArray<FString>& Examples)
{
	FString ErrorMessage;

	// Error header with emoji/icon
	switch (ErrorType)
	{
	case EErrorType::InvalidParameters:
		ErrorMessage = TEXT("❌ INVALID PARAMETERS\n");
		break;
	case EErrorType::NoSelection:
		ErrorMessage = TEXT("⚠️  NO SELECTION\n");
		break;
	case EErrorType::AssetNotFound:
		ErrorMessage = TEXT("🔍 ASSET NOT FOUND\n");
		break;
	case EErrorType::InvalidAssetType:
		ErrorMessage = TEXT("⚠️  INVALID ASSET TYPE\n");
		break;
	case EErrorType::OperationFailed:
		ErrorMessage = TEXT("❌ OPERATION FAILED\n");
		break;
	case EErrorType::UnsupportedOperation:
		ErrorMessage = TEXT("⚠️  UNSUPPORTED OPERATION\n");
		break;
	case EErrorType::PermissionDenied:
		ErrorMessage = TEXT("🔒 PERMISSION DENIED\n");
		break;
	}

	// Context
	ErrorMessage += FString::Printf(TEXT("\n%s\n"), *Context);

	// Suggestions
	if (Suggestions.Num() > 0)
	{
		ErrorMessage += FormatSuggestions(Suggestions);
	}

	// Examples
	if (Examples.Num() > 0)
	{
		ErrorMessage += FormatExamples(Examples);
	}

	return ErrorMessage;
}

FString FErrorMessageSystem::NoActorsSelected(const FString& OperationName)
{
	TArray<FString> Suggestions = {
		TEXT("Select one or more actors in the viewport"),
		TEXT("Click actors while holding Ctrl to multi-select"),
		TEXT("Use box select (drag) to select multiple actors")
	};

	TArray<FString> Examples = {
		FString::Printf(TEXT("1. Select actors in viewport")),
		FString::Printf(TEXT("2. Try: '%s'"), *OperationName),
	};

	return GetEnhancedError(
		EErrorType::NoSelection,
		FString::Printf(TEXT("Operation '%s' requires actors to be selected."), *OperationName),
		Suggestions,
		Examples
	);
}

FString FErrorMessageSystem::AssetNotFound(const FString& AssetQuery, const TArray<FString>& SimilarAssets)
{
	TArray<FString> Suggestions;

	Suggestions.Add(TEXT("Check spelling of asset name"));
	Suggestions.Add(TEXT("Try a more general search term"));
	Suggestions.Add(TEXT("Use asset catalog: 'What assets are available?'"));

	if (SimilarAssets.Num() > 0)
	{
		Suggestions.Add(TEXT("Did you mean one of these?"));
		for (const FString& Asset : SimilarAssets)
		{
			Suggestions.Add(FString::Printf(TEXT("  - %s"), *Asset));
		}
	}

	TArray<FString> Examples = {
		TEXT("Try: 'Place a building' (generic)"),
		TEXT("Or: 'Place a French farmhouse' (specific)"),
		TEXT("Or: 'What French assets are available?' (discovery)")
	};

	return GetEnhancedError(
		EErrorType::AssetNotFound,
		FString::Printf(TEXT("Could not find asset matching: '%s'"), *AssetQuery),
		Suggestions,
		Examples
	);
}

FString FErrorMessageSystem::InvalidParameters(const FString& CommandName, const TArray<FString>& RequiredParams)
{
	TArray<FString> Suggestions;

	Suggestions.Add(FString::Printf(TEXT("Command '%s' requires:"), *CommandName));
	for (const FString& Param : RequiredParams)
	{
		Suggestions.Add(FString::Printf(TEXT("  - %s"), *Param));
	}

	Suggestions.Add(TEXT(""));
	Suggestions.Add(TEXT("Use 'Help' button for complete command reference"));

	// Get related commands
	InitializeCommandRelationships();
	TArray<FString> RelatedCommands = GetRelatedCommands(CommandName);
	if (RelatedCommands.Num() > 0)
	{
		Suggestions.Add(TEXT(""));
		Suggestions.Add(TEXT("Related commands you might want:"));
		for (const FString& Related : RelatedCommands)
		{
			Suggestions.Add(FString::Printf(TEXT("  - %s"), *Related));
		}
	}

	TArray<FString> Examples;
	if (CommandName == TEXT("spawn_object"))
	{
		Examples.Add(TEXT("'Place a French building at camera location'"));
		Examples.Add(TEXT("'Spawn a tree at 1000, 500, 0'"));
	}
	else if (CommandName == TEXT("place_props"))
	{
		Examples.Add(TEXT("'Place 20 sandbags in a line here'"));
		Examples.Add(TEXT("'Scatter 50 rocks in this area'"));
	}

	return GetEnhancedError(
		EErrorType::InvalidParameters,
		FString::Printf(TEXT("Invalid or missing parameters for command: '%s'"), *CommandName),
		Suggestions,
		Examples
	);
}

FString FErrorMessageSystem::OperationFailed(const FString& OperationName, const FString& Reason, const TArray<FString>& Solutions)
{
	TArray<FString> Suggestions;

	Suggestions.Add(FString::Printf(TEXT("Reason: %s"), *Reason));
	Suggestions.Add(TEXT(""));

	if (Solutions.Num() > 0)
	{
		Suggestions.Add(TEXT("Possible solutions:"));
		for (const FString& Solution : Solutions)
		{
			Suggestions.Add(FString::Printf(TEXT("  - %s"), *Solution));
		}
	}
	else
	{
		Suggestions.Add(TEXT("Try:"));
		Suggestions.Add(TEXT("  - Check Project Settings → Claude Terminal"));
		Suggestions.Add(TEXT("  - Verify assets exist in project"));
		Suggestions.Add(TEXT("  - Check UE5 output log for details"));
	}

	return GetEnhancedError(
		EErrorType::OperationFailed,
		FString::Printf(TEXT("Operation '%s' failed"), *OperationName),
		Suggestions,
		{}
	);
}

FString FErrorMessageSystem::UnsupportedAssetType(const FString& AssetType, const TArray<FString>& SupportedTypes)
{
	TArray<FString> Suggestions;

	Suggestions.Add(FString::Printf(TEXT("Asset type '%s' is not supported by this operation"), *AssetType));
	Suggestions.Add(TEXT(""));
	Suggestions.Add(TEXT("Supported types:"));

	for (const FString& Type : SupportedTypes)
	{
		Suggestions.Add(FString::Printf(TEXT("  - %s"), *Type));
	}

	return GetEnhancedError(
		EErrorType::InvalidAssetType,
		FString::Printf(TEXT("Unsupported asset type: %s"), *AssetType),
		Suggestions,
		{}
	);
}

TArray<FString> FErrorMessageSystem::GetRelatedCommands(const FString& CommandName)
{
	InitializeCommandRelationships();

	if (const TArray<FString>* Related = CommandRelationships.Find(CommandName))
	{
		return *Related;
	}

	return {};
}

FString FErrorMessageSystem::FormatSuggestions(const TArray<FString>& Suggestions)
{
	FString Result = TEXT("\n💡 SUGGESTIONS:\n");

	for (const FString& Suggestion : Suggestions)
	{
		Result += FString::Printf(TEXT("   %s\n"), *Suggestion);
	}

	return Result;
}

FString FErrorMessageSystem::FormatExamples(const TArray<FString>& Examples)
{
	FString Result = TEXT("\n📋 EXAMPLES:\n");

	for (const FString& Example : Examples)
	{
		Result += FString::Printf(TEXT("   %s\n"), *Example);
	}

	return Result;
}
