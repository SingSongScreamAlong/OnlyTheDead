// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"

DECLARE_DELEGATE_OneParam(FOnPreviewConfirmed, const FTransform&);
DECLARE_DELEGATE(FOnPreviewCancelled);

/**
 * Asset preview system for showing ghost meshes before placement
 * TIER 2.2 Feature
 */
class CLAUDETERMINALEDITOR_API FAssetPreviewSystem
{
public:
	FAssetPreviewSystem();
	~FAssetPreviewSystem();

	/**
	 * Start previewing an asset for placement
	 * @param AssetPath Path to the asset to preview
	 * @param InitialLocation Starting location for preview
	 * @param World World to spawn preview in
	 * @param OnConfirmed Callback when user confirms placement
	 * @param OnCancelled Callback when user cancels
	 * @return true if preview started successfully
	 */
	bool StartPreview(
		const FString& AssetPath,
		const FVector& InitialLocation,
		UWorld* World,
		FOnPreviewConfirmed OnConfirmed,
		FOnPreviewCancelled OnCancelled);

	/**
	 * Stop current preview
	 */
	void StopPreview();

	/**
	 * Check if preview is active
	 */
	bool IsPreviewActive() const { return PreviewActor != nullptr; }

	/**
	 * Update preview location (from mouse movement)
	 */
	void UpdatePreviewLocation(const FVector& NewLocation);

	/**
	 * Rotate preview actor
	 * @param DeltaRotation Rotation to apply (in degrees)
	 */
	void RotatePreview(float DeltaRotation);

	/**
	 * Scale preview actor
	 * @param ScaleMultiplier Scale multiplier (1.0 = normal, 2.0 = double size)
	 */
	void ScalePreview(float ScaleMultiplier);

	/**
	 * Confirm current preview placement
	 */
	void ConfirmPreview();

	/**
	 * Cancel current preview
	 */
	void CancelPreview();

	/**
	 * Get current preview transform
	 */
	FTransform GetPreviewTransform() const;

private:
	// Create preview actor from asset
	bool CreatePreviewActor(const FString& AssetPath, const FVector& Location, UWorld* World);

	// Apply ghost material to preview
	void ApplyGhostMaterial();

	// Clean up preview actor
	void CleanupPreviewActor();

	// Preview state
	AActor* PreviewActor;
	UWorld* PreviewWorld;
	FString PreviewAssetPath;
	FVector PreviewScale;
	FRotator PreviewRotation;

	// Delegates
	FOnPreviewConfirmed OnConfirmedCallback;
	FOnPreviewCancelled OnCancelledCallback;

	// Ghost material
	UMaterialInterface* GhostMaterial;
};
