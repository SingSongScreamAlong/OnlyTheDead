// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Blueprint.h"

/**
 * Asset metadata for semantic search and cataloging
 */
struct FAssetMetadata
{
	// Asset reference
	FString AssetPath;
	FString AssetName;
	FString AssetType; // StaticMesh, Material, Blueprint, etc.

	// Semantic information
	TArray<FString> Tags; // e.g., "french", "building", "wwi", "destroyed"
	TArray<FString> Categories; // e.g., "Architecture", "Props", "Vehicles"
	FString Description;
	FString Era; // e.g., "WWI", "1916", "Early 20th Century"
	FString Region; // e.g., "France", "Verdun", "Europe"

	// Physical properties
	FVector ApproximateSize;
	bool bIsDestructible;
	bool bIsDamaged;

	// Usage hints
	int32 UseCount; // How many times used in scene
	float RelevanceScore; // For search ranking
};

/**
 * Catalogs and indexes all assets in the project for intelligent selection
 * Enables semantic search: "French WWI building" → finds appropriate assets
 */
class CLAUDETERMINALEDITOR_API FAssetCatalogSystem
{
public:
	FAssetCatalogSystem();
	~FAssetCatalogSystem();

	/**
	 * Scan project and build asset catalog
	 * @param OutLog Execution log
	 * @return true if successful
	 */
	bool BuildCatalog(FString& OutLog);

	/**
	 * Search for assets matching semantic query
	 * @param Query Natural language query (e.g., "French WWI farmhouse")
	 * @param OutAssets Matching assets, sorted by relevance
	 * @param MaxResults Maximum number of results to return
	 * @return Number of matches found
	 */
	int32 SearchAssets(const FString& Query, TArray<FAssetMetadata>& OutAssets, int32 MaxResults = 10);

	/**
	 * Search for specific asset type with semantic query
	 */
	int32 SearchStaticMeshes(const FString& Query, TArray<FAssetMetadata>& OutAssets, int32 MaxResults = 10);
	int32 SearchMaterials(const FString& Query, TArray<FAssetMetadata>& OutAssets, int32 MaxResults = 10);
	int32 SearchBlueprints(const FString& Query, TArray<FAssetMetadata>& OutAssets, int32 MaxResults = 10);

	/**
	 * Get asset catalog as string (for Claude's context)
	 * @param bIncludeDetails Include detailed metadata
	 * @return Formatted catalog string
	 */
	FString GetCatalogSummary(bool bIncludeDetails = false) const;

	/**
	 * Update asset metadata (e.g., after placing in scene)
	 */
	void UpdateAssetUsage(const FString& AssetPath);

	/**
	 * Get total number of cataloged assets
	 */
	int32 GetAssetCount() const { return AssetCatalog.Num(); }

	/**
	 * Check if catalog is built
	 */
	bool IsCatalogReady() const { return bCatalogBuilt; }

	/**
	 * Get assets by category
	 */
	TArray<FAssetMetadata> GetAssetsByCategory(const FString& Category) const;

	/**
	 * Get assets by era
	 */
	TArray<FAssetMetadata> GetAssetsByEra(const FString& Era) const;

	// ===== AUTO-REFRESH FUNCTIONALITY (TIER 1.3) =====

	/**
	 * Enable or disable automatic catalog refresh when assets change
	 * @param bEnable true to enable auto-refresh
	 */
	void EnableAutoRefresh(bool bEnable);

	/**
	 * Check if auto-refresh is currently enabled
	 */
	bool IsAutoRefreshEnabled() const { return bAutoRefreshEnabled; }

	/**
	 * Manually rebuild the entire catalog
	 */
	void RebuildCatalog(FString& OutLog);

	/**
	 * Perform incremental update (adds/removes single asset without full rebuild)
	 */
	void IncrementalUpdate();

private:
	// Scan content directories
	void ScanContentDirectory(const FString& Directory, FString& OutLog);

	// Analyze asset and extract metadata
	FAssetMetadata AnalyzeAsset(const FString& AssetPath);

	// Extract semantic information from asset name/path
	void ExtractSemanticInfo(FAssetMetadata& Metadata);

	// Calculate relevance score for search query
	float CalculateRelevance(const FAssetMetadata& Asset, const FString& Query) const;

	// Parse semantic query into keywords
	TArray<FString> ParseQuery(const FString& Query) const;

	// Auto-refresh delegate handlers
	void OnAssetAdded(const FAssetData& AssetData);
	void OnAssetRemoved(const FAssetData& AssetData);
	void OnAssetRenamed(const FAssetData& AssetData, const FString& OldPath);

	// Watch Asset Registry for changes
	void WatchAssetRegistry();
	void UnwatchAssetRegistry();

	// Add single asset to catalog
	void AddAssetToCatalog(const FAssetData& AssetData);

	// Remove single asset from catalog
	void RemoveAssetFromCatalog(const FString& AssetPath);

	// Asset database
	TArray<FAssetMetadata> AssetCatalog;
	TMap<FString, int32> AssetPathToIndex; // Fast lookup
	bool bCatalogBuilt;

	// Semantic keyword mappings
	TMap<FString, TArray<FString>> KeywordAliases; // "house" → ["building", "structure", "farmhouse"]

	// Auto-refresh state
	bool bAutoRefreshEnabled;
	FDelegateHandle AssetAddedHandle;
	FDelegateHandle AssetRemovedHandle;
	FDelegateHandle AssetRenamedHandle;

	// Thread safety for catalog modifications
	mutable FCriticalSection CatalogMutex;
};
