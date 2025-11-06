// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetCatalogSystem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Engine/Blueprint.h"
#include "UObject/ConstructorHelpers.h"

FAssetCatalogSystem::FAssetCatalogSystem()
	: bCatalogBuilt(false)
{
	// Initialize keyword aliases for semantic search
	KeywordAliases.Add(TEXT("building"), {TEXT("structure"), TEXT("house"), TEXT("farmhouse"), TEXT("church"), TEXT("fort")});
	KeywordAliases.Add(TEXT("house"), {TEXT("building"), TEXT("farmhouse"), TEXT("cottage"), TEXT("dwelling")});
	KeywordAliases.Add(TEXT("vehicle"), {TEXT("car"), TEXT("truck"), TEXT("tank"), TEXT("transport")});
	KeywordAliases.Add(TEXT("weapon"), {TEXT("gun"), TEXT("rifle"), TEXT("artillery"), TEXT("cannon")});
	KeywordAliases.Add(TEXT("tree"), {TEXT("forest"), TEXT("vegetation"), TEXT("woods"), TEXT("foliage")});
	KeywordAliases.Add(TEXT("rock"), {TEXT("stone"), TEXT("boulder"), TEXT("rubble"), TEXT("debris")});
	KeywordAliases.Add(TEXT("french"), {TEXT("france"), TEXT("verdun"), TEXT("french")});
	KeywordAliases.Add(TEXT("german"), {TEXT("germany"), TEXT("prussian"), TEXT("kaiser")});
	KeywordAliases.Add(TEXT("damaged"), {TEXT("destroyed"), TEXT("ruined"), TEXT("broken"), TEXT("wrecked")});
	KeywordAliases.Add(TEXT("wwi"), {TEXT("ww1"), TEXT("1914"), TEXT("1915"), TEXT("1916"), TEXT("1917"), TEXT("1918"), TEXT("greatwar")});
}

FAssetCatalogSystem::~FAssetCatalogSystem()
{
}

bool FAssetCatalogSystem::BuildCatalog(FString& OutLog)
{
	OutLog = TEXT("Building asset catalog...\n");

	AssetCatalog.Empty();
	AssetPathToIndex.Empty();

	// Get Asset Registry
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Ensure asset registry is loaded
	AssetRegistry.SearchAllAssets(true);

	int32 TotalAssets = 0;

	// Scan for Static Meshes
	{
		TArray<FAssetData> MeshAssets;
		AssetRegistry.GetAssetsByClass(UStaticMesh::StaticClass()->GetClassPathName(), MeshAssets);

		for (const FAssetData& AssetData : MeshAssets)
		{
			FAssetMetadata Metadata = AnalyzeAsset(AssetData.GetObjectPathString());
			Metadata.AssetType = TEXT("StaticMesh");

			AssetPathToIndex.Add(Metadata.AssetPath, AssetCatalog.Num());
			AssetCatalog.Add(Metadata);
			TotalAssets++;
		}

		OutLog += FString::Printf(TEXT("  Found %d static meshes\n"), MeshAssets.Num());
	}

	// Scan for Materials
	{
		TArray<FAssetData> MaterialAssets;
		AssetRegistry.GetAssetsByClass(UMaterialInterface::StaticClass()->GetClassPathName(), MaterialAssets);

		for (const FAssetData& AssetData : MaterialAssets)
		{
			FAssetMetadata Metadata = AnalyzeAsset(AssetData.GetObjectPathString());
			Metadata.AssetType = TEXT("Material");

			AssetPathToIndex.Add(Metadata.AssetPath, AssetCatalog.Num());
			AssetCatalog.Add(Metadata);
			TotalAssets++;
		}

		OutLog += FString::Printf(TEXT("  Found %d materials\n"), MaterialAssets.Num());
	}

	// Scan for Blueprints
	{
		TArray<FAssetData> BlueprintAssets;
		AssetRegistry.GetAssetsByClass(UBlueprint::StaticClass()->GetClassPathName(), BlueprintAssets);

		for (const FAssetData& AssetData : BlueprintAssets)
		{
			FAssetMetadata Metadata = AnalyzeAsset(AssetData.GetObjectPathString());
			Metadata.AssetType = TEXT("Blueprint");

			AssetPathToIndex.Add(Metadata.AssetPath, AssetCatalog.Num());
			AssetCatalog.Add(Metadata);
			TotalAssets++;
		}

		OutLog += FString::Printf(TEXT("  Found %d blueprints\n"), BlueprintAssets.Num());
	}

	bCatalogBuilt = true;
	OutLog += FString::Printf(TEXT("Asset catalog built: %d total assets\n"), TotalAssets);

	return true;
}

FAssetMetadata FAssetCatalogSystem::AnalyzeAsset(const FString& AssetPath)
{
	FAssetMetadata Metadata;
	Metadata.AssetPath = AssetPath;

	// Extract asset name from path
	FString LeftStr, RightStr;
	if (AssetPath.Split(TEXT("."), &LeftStr, &RightStr, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
	{
		Metadata.AssetName = RightStr;
	}
	else
	{
		Metadata.AssetName = AssetPath;
	}

	// Extract semantic information from name and path
	ExtractSemanticInfo(Metadata);

	return Metadata;
}

void FAssetCatalogSystem::ExtractSemanticInfo(FAssetMetadata& Metadata)
{
	FString LowerName = Metadata.AssetName.ToLower();
	FString LowerPath = Metadata.AssetPath.ToLower();

	// Extract tags from name and path
	TArray<FString> PossibleTags = {
		TEXT("french"), TEXT("german"), TEXT("british"), TEXT("american"),
		TEXT("building"), TEXT("house"), TEXT("church"), TEXT("fort"), TEXT("bunker"),
		TEXT("tree"), TEXT("rock"), TEXT("vegetation"), TEXT("foliage"),
		TEXT("vehicle"), TEXT("tank"), TEXT("truck"), TEXT("car"),
		TEXT("weapon"), TEXT("rifle"), TEXT("artillery"), TEXT("cannon"),
		TEXT("trench"), TEXT("barbed"), TEXT("wire"), TEXT("sandbag"),
		TEXT("damaged"), TEXT("destroyed"), TEXT("ruined"), TEXT("broken"),
		TEXT("wwi"), TEXT("ww1"), TEXT("1916"), TEXT("verdun")
	};

	for (const FString& Tag : PossibleTags)
	{
		if (LowerName.Contains(Tag.ToLower()) || LowerPath.Contains(Tag.ToLower()))
		{
			Metadata.Tags.AddUnique(Tag);
		}
	}

	// Extract era
	if (LowerName.Contains(TEXT("wwi")) || LowerName.Contains(TEXT("ww1")) ||
	    LowerName.Contains(TEXT("1914")) || LowerName.Contains(TEXT("1915")) ||
	    LowerName.Contains(TEXT("1916")) || LowerName.Contains(TEXT("1917")) ||
	    LowerName.Contains(TEXT("1918")))
	{
		Metadata.Era = TEXT("WWI");
	}

	// Extract region
	if (LowerName.Contains(TEXT("french")) || LowerPath.Contains(TEXT("french")) ||
	    LowerName.Contains(TEXT("verdun")) || LowerPath.Contains(TEXT("verdun")))
	{
		Metadata.Region = TEXT("France");
	}
	else if (LowerName.Contains(TEXT("german")) || LowerPath.Contains(TEXT("german")))
	{
		Metadata.Region = TEXT("Germany");
	}

	// Extract categories
	if (LowerName.Contains(TEXT("building")) || LowerName.Contains(TEXT("house")) ||
	    LowerName.Contains(TEXT("church")) || LowerName.Contains(TEXT("fort")))
	{
		Metadata.Categories.AddUnique(TEXT("Architecture"));
	}
	if (LowerName.Contains(TEXT("tree")) || LowerName.Contains(TEXT("vegetation")) ||
	    LowerName.Contains(TEXT("foliage")))
	{
		Metadata.Categories.AddUnique(TEXT("Nature"));
	}
	if (LowerName.Contains(TEXT("vehicle")) || LowerName.Contains(TEXT("tank")) ||
	    LowerName.Contains(TEXT("truck")))
	{
		Metadata.Categories.AddUnique(TEXT("Vehicles"));
	}
	if (LowerName.Contains(TEXT("weapon")) || LowerName.Contains(TEXT("rifle")) ||
	    LowerName.Contains(TEXT("artillery")))
	{
		Metadata.Categories.AddUnique(TEXT("Weapons"));
	}

	// Determine damage state
	Metadata.bIsDamaged = LowerName.Contains(TEXT("damaged")) ||
	                       LowerName.Contains(TEXT("destroyed")) ||
	                       LowerName.Contains(TEXT("ruined"));
}

int32 FAssetCatalogSystem::SearchAssets(const FString& Query, TArray<FAssetMetadata>& OutAssets, int32 MaxResults)
{
	OutAssets.Empty();

	if (!bCatalogBuilt)
	{
		return 0;
	}

	TArray<FString> Keywords = ParseQuery(Query);

	// Calculate relevance for each asset
	TArray<TPair<float, int32>> ScoredAssets; // Score, Index

	for (int32 i = 0; i < AssetCatalog.Num(); i++)
	{
		float Score = CalculateRelevance(AssetCatalog[i], Query);
		if (Score > 0.0f)
		{
			ScoredAssets.Add(TPair<float, int32>(Score, i));
		}
	}

	// Sort by score (highest first)
	ScoredAssets.Sort([](const TPair<float, int32>& A, const TPair<float, int32>& B) {
		return A.Key > B.Key;
	});

	// Return top results
	int32 NumResults = FMath::Min(ScoredAssets.Num(), MaxResults);
	for (int32 i = 0; i < NumResults; i++)
	{
		OutAssets.Add(AssetCatalog[ScoredAssets[i].Value]);
	}

	return OutAssets.Num();
}

int32 FAssetCatalogSystem::SearchStaticMeshes(const FString& Query, TArray<FAssetMetadata>& OutAssets, int32 MaxResults)
{
	TArray<FAssetMetadata> AllResults;
	SearchAssets(Query, AllResults, 1000);

	OutAssets.Empty();
	for (const FAssetMetadata& Asset : AllResults)
	{
		if (Asset.AssetType == TEXT("StaticMesh"))
		{
			OutAssets.Add(Asset);
			if (OutAssets.Num() >= MaxResults)
				break;
		}
	}

	return OutAssets.Num();
}

int32 FAssetCatalogSystem::SearchMaterials(const FString& Query, TArray<FAssetMetadata>& OutAssets, int32 MaxResults)
{
	TArray<FAssetMetadata> AllResults;
	SearchAssets(Query, AllResults, 1000);

	OutAssets.Empty();
	for (const FAssetMetadata& Asset : AllResults)
	{
		if (Asset.AssetType == TEXT("Material"))
		{
			OutAssets.Add(Asset);
			if (OutAssets.Num() >= MaxResults)
				break;
		}
	}

	return OutAssets.Num();
}

int32 FAssetCatalogSystem::SearchBlueprints(const FString& Query, TArray<FAssetMetadata>& OutAssets, int32 MaxResults)
{
	TArray<FAssetMetadata> AllResults;
	SearchAssets(Query, AllResults, 1000);

	OutAssets.Empty();
	for (const FAssetMetadata& Asset : AllResults)
	{
		if (Asset.AssetType == TEXT("Blueprint"))
		{
			OutAssets.Add(Asset);
			if (OutAssets.Num() >= MaxResults)
				break;
		}
	}

	return OutAssets.Num();
}

FString FAssetCatalogSystem::GetCatalogSummary(bool bIncludeDetails) const
{
	FString Summary = FString::Printf(TEXT("Asset Catalog: %d total assets\n"), AssetCatalog.Num());

	// Count by type
	TMap<FString, int32> TypeCounts;
	TMap<FString, int32> CategoryCounts;

	for (const FAssetMetadata& Asset : AssetCatalog)
	{
		TypeCounts.FindOrAdd(Asset.AssetType)++;
		for (const FString& Category : Asset.Categories)
		{
			CategoryCounts.FindOrAdd(Category)++;
		}
	}

	Summary += TEXT("\nBy Type:\n");
	for (const auto& Pair : TypeCounts)
	{
		Summary += FString::Printf(TEXT("  %s: %d\n"), *Pair.Key, Pair.Value);
	}

	Summary += TEXT("\nBy Category:\n");
	for (const auto& Pair : CategoryCounts)
	{
		Summary += FString::Printf(TEXT("  %s: %d\n"), *Pair.Key, Pair.Value);
	}

	if (bIncludeDetails)
	{
		Summary += TEXT("\nSample Assets:\n");
		int32 SampleCount = FMath::Min(20, AssetCatalog.Num());
		for (int32 i = 0; i < SampleCount; i++)
		{
			const FAssetMetadata& Asset = AssetCatalog[i];
			Summary += FString::Printf(TEXT("  %s (%s)"), *Asset.AssetName, *Asset.AssetType);
			if (Asset.Tags.Num() > 0)
			{
				Summary += TEXT(" - Tags: ");
				for (int32 j = 0; j < FMath::Min(3, Asset.Tags.Num()); j++)
				{
					Summary += Asset.Tags[j];
					if (j < FMath::Min(3, Asset.Tags.Num()) - 1)
						Summary += TEXT(", ");
				}
			}
			Summary += TEXT("\n");
		}
	}

	return Summary;
}

void FAssetCatalogSystem::UpdateAssetUsage(const FString& AssetPath)
{
	if (int32* IndexPtr = AssetPathToIndex.Find(AssetPath))
	{
		AssetCatalog[*IndexPtr].UseCount++;
	}
}

TArray<FAssetMetadata> FAssetCatalogSystem::GetAssetsByCategory(const FString& Category) const
{
	TArray<FAssetMetadata> Results;
	for (const FAssetMetadata& Asset : AssetCatalog)
	{
		if (Asset.Categories.Contains(Category))
		{
			Results.Add(Asset);
		}
	}
	return Results;
}

TArray<FAssetMetadata> FAssetCatalogSystem::GetAssetsByEra(const FString& Era) const
{
	TArray<FAssetMetadata> Results;
	for (const FAssetMetadata& Asset : AssetCatalog)
	{
		if (Asset.Era == Era)
		{
			Results.Add(Asset);
		}
	}
	return Results;
}

float FAssetCatalogSystem::CalculateRelevance(const FAssetMetadata& Asset, const FString& Query) const
{
	float Score = 0.0f;
	FString LowerQuery = Query.ToLower();
	FString LowerName = Asset.AssetName.ToLower();

	// Direct name match (highest weight)
	if (LowerName.Contains(LowerQuery))
	{
		Score += 10.0f;
	}

	// Parse query into keywords
	TArray<FString> Keywords = ParseQuery(Query);

	for (const FString& Keyword : Keywords)
	{
		FString LowerKeyword = Keyword.ToLower();

		// Tag match (high weight)
		for (const FString& Tag : Asset.Tags)
		{
			if (Tag.ToLower().Contains(LowerKeyword))
			{
				Score += 5.0f;
			}
		}

		// Category match (medium weight)
		for (const FString& Category : Asset.Categories)
		{
			if (Category.ToLower().Contains(LowerKeyword))
			{
				Score += 3.0f;
			}
		}

		// Era match (medium weight)
		if (!Asset.Era.IsEmpty() && Asset.Era.ToLower().Contains(LowerKeyword))
		{
			Score += 3.0f;
		}

		// Region match (medium weight)
		if (!Asset.Region.IsEmpty() && Asset.Region.ToLower().Contains(LowerKeyword))
		{
			Score += 3.0f;
		}

		// Name partial match (low weight)
		if (LowerName.Contains(LowerKeyword))
		{
			Score += 2.0f;
		}

		// Check keyword aliases
		if (const TArray<FString>* Aliases = KeywordAliases.Find(LowerKeyword))
		{
			for (const FString& Alias : *Aliases)
			{
				if (LowerName.Contains(Alias.ToLower()))
				{
					Score += 1.0f;
				}
			}
		}
	}

	// Boost frequently used assets slightly
	Score += Asset.UseCount * 0.1f;

	return Score;
}

TArray<FString> FAssetCatalogSystem::ParseQuery(const FString& Query) const
{
	TArray<FString> Keywords;

	// Split by spaces
	FString TrimmedQuery = Query.TrimStartAndEnd();
	TrimmedQuery.ParseIntoArray(Keywords, TEXT(" "), true);

	// Remove common words
	TArray<FString> StopWords = {TEXT("a"), TEXT("an"), TEXT("the"), TEXT("at"), TEXT("in"), TEXT("on"), TEXT("with")};
	Keywords = Keywords.FilterByPredicate([&StopWords](const FString& Word) {
		return !StopWords.Contains(Word.ToLower());
	});

	return Keywords;
}
