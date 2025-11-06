// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

/**
 * Smart placement options
 */
struct FSmartPlacementOptions
{
	// Check for collisions and prevent overlaps
	bool bAvoidCollisions = true;

	// Snap to surfaces (walls, floors, ceilings)
	bool bSnapToSurface = true;

	// Align with surface normal
	bool bAlignWithSurface = false;

	// Use physics simulation for placement
	bool bUsePhysicsPlacement = false;

	// Minimum clearance around placed object
	float MinimumClearance = 50.0f;

	// Maximum slope angle for placement (degrees)
	float MaxSlopeAngle = 45.0f;

	// Snap distance threshold
	float SnapDistance = 100.0f;
};

/**
 * Smart placement result
 */
struct FSmartPlacementResult
{
	bool bSuccess = false;
	FVector AdjustedLocation = FVector::ZeroVector;
	FRotator AdjustedRotation = FRotator::ZeroRotator;
	FString FailureReason;
	bool bLocationAdjusted = false;
	bool bRotationAdjusted = false;
};

/**
 * Intelligent placement system with collision avoidance and surface snapping
 * TIER 2.3 Feature
 */
class CLAUDETERMINALEDITOR_API FSmartPlacementSystem
{
public:
	/**
	 * Find optimal placement location with smart adjustments
	 * @param DesiredLocation User's desired location
	 * @param DesiredRotation Desired rotation
	 * @param ActorToPlace Actor being placed (for bounds calculation)
	 * @param World World to place in
	 * @param Options Placement options
	 * @return Placement result with adjusted transform
	 */
	static FSmartPlacementResult FindOptimalPlacement(
		const FVector& DesiredLocation,
		const FRotator& DesiredRotation,
		AActor* ActorToPlace,
		UWorld* World,
		const FSmartPlacementOptions& Options);

	/**
	 * Check if location is valid for placement
	 * @param Location Location to check
	 * @param ActorBounds Bounds of actor being placed
	 * @param World World context
	 * @param Options Placement options
	 * @return true if location is valid
	 */
	static bool IsLocationValid(
		const FVector& Location,
		const FBox& ActorBounds,
		UWorld* World,
		const FSmartPlacementOptions& Options);

	/**
	 * Snap location to nearest surface
	 * @param Location Starting location
	 * @param World World context
	 * @param SnapDistance Maximum snap distance
	 * @param OutSurfaceNormal Surface normal at snap point
	 * @return Snapped location
	 */
	static FVector SnapToSurface(
		const FVector& Location,
		UWorld* World,
		float SnapDistance,
		FVector& OutSurfaceNormal);

	/**
	 * Align rotation with surface normal
	 * @param BaseRotation Starting rotation
	 * @param SurfaceNormal Surface normal to align with
	 * @return Adjusted rotation
	 */
	static FRotator AlignWithSurface(const FRotator& BaseRotation, const FVector& SurfaceNormal);

	/**
	 * Check for collision overlaps
	 * @param Location Location to check
	 * @param ActorBounds Actor bounds
	 * @param World World context
	 * @return true if collision detected
	 */
	static bool CheckCollision(const FVector& Location, const FBox& ActorBounds, UWorld* World);

	/**
	 * Find nearest valid placement location
	 * @param StartLocation Starting point
	 * @param ActorBounds Actor bounds
	 * @param World World context
	 * @param SearchRadius Search radius
	 * @param Options Placement options
	 * @return Valid location or StartLocation if none found
	 */
	static FVector FindNearestValidLocation(
		const FVector& StartLocation,
		const FBox& ActorBounds,
		UWorld* World,
		float SearchRadius,
		const FSmartPlacementOptions& Options);

	/**
	 * Calculate slope angle at location
	 * @param Location Location to check
	 * @param World World context
	 * @return Slope angle in degrees
	 */
	static float CalculateSlopeAngle(const FVector& Location, UWorld* World);
};
