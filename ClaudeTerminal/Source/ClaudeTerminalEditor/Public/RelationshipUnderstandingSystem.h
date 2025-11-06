// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

/**
 * Spatial relationship types
 */
UENUM()
enum class ESpatialRelationship : uint8
{
	None,
	Around,          // "Place fence around building"
	Between,         // "Place between these two points"
	Facing,          // "Place facing this direction"
	NextTo,          // "Place next to the building"
	Above,           // "Place above the ground"
	Below,           // "Place below the ceiling"
	Inside,          // "Place inside the room"
	DefendingFrom,   // "Place sandbags defending from that direction"
	Connecting,      // "Create trench connecting these points"
	AlongPath,       // "Place trees along the road"
	Scattered,       // "Scatter around this area"
	InFrontOf,       // "Place in front of the building"
	Behind,          // "Place behind the wall"
	OnTopOf          // "Place on top of the platform"
};

/**
 * Relationship context - describes spatial relationship between objects
 */
struct FRelationshipContext
{
	ESpatialRelationship Relationship = ESpatialRelationship::None;
	TArray<AActor*> ReferenceActors;    // Actors being referenced
	TArray<FVector> ReferencePoints;     // Specific points
	FVector Direction = FVector::ZeroVector;  // Direction vector (for "facing", "defending from")
	float Distance = 100.0f;              // Distance parameter
	bool bExcludeZone = false;            // Exclude zone (e.g., "around but not on")
	FString RawQuery;                     // Original query string
};

/**
 * Placement result for relationship-based positioning
 */
struct FRelationshipPlacementResult
{
	bool bSuccess = false;
	TArray<FVector> PlacementLocations;
	TArray<FRotator> PlacementRotations;
	FString ErrorMessage;
};

/**
 * System for understanding and executing spatial relationships
 * TIER 2.4 Feature
 */
class CLAUDETERMINALEDITOR_API FRelationshipUnderstandingSystem
{
public:
	/**
	 * Parse natural language query for spatial relationships
	 * @param Query Natural language query (e.g., "place around this building")
	 * @param ContextActors Currently selected actors for context
	 * @return Parsed relationship context
	 */
	static FRelationshipContext ParseRelationship(const FString& Query, const TArray<AActor*>& ContextActors);

	/**
	 * Calculate placement locations based on relationship
	 * @param Context Relationship context
	 * @param Count Number of objects to place
	 * @param World World context
	 * @return Placement result with locations and rotations
	 */
	static FRelationshipPlacementResult CalculatePlacement(
		const FRelationshipContext& Context,
		int32 Count,
		UWorld* World);

	/**
	 * Generate circular pattern around reference point/object
	 * @param Center Center point
	 * @param Radius Radius of circle
	 * @param Count Number of points
	 * @param bFaceCenter Should objects face inward?
	 * @return Array of transforms
	 */
	static TArray<FTransform> GenerateCircularPattern(
		const FVector& Center,
		float Radius,
		int32 Count,
		bool bFaceCenter = false);

	/**
	 * Generate perimeter pattern around bounding box
	 * @param Bounds Bounding box to surround
	 * @param Spacing Spacing between objects
	 * @param Offset Distance from bounds edge
	 * @return Array of transforms
	 */
	static TArray<FTransform> GeneratePerimeterPattern(
		const FBox& Bounds,
		float Spacing,
		float Offset = 100.0f);

	/**
	 * Generate defensive arc pattern
	 * @param DefensePoint Point being defended
	 * @param ThreatDirection Direction of threat
	 * @param Count Number of defensive positions
	 * @param ArcAngle Total arc angle in degrees
	 * @return Array of transforms facing threat direction
	 */
	static TArray<FTransform> GenerateDefensiveArc(
		const FVector& DefensePoint,
		const FVector& ThreatDirection,
		int32 Count,
		float ArcAngle = 180.0f);

	/**
	 * Generate path between points
	 * @param StartPoint Start location
	 * @param EndPoint End location
	 * @param Count Number of objects along path
	 * @param bAlignWithPath Should objects rotate along path?
	 * @return Array of transforms
	 */
	static TArray<FTransform> GeneratePathPattern(
		const FVector& StartPoint,
		const FVector& EndPoint,
		int32 Count,
		bool bAlignWithPath = true);

	/**
	 * Generate scattered pattern in area
	 * @param Center Center of area
	 * @param Radius Radius of area
	 * @param Count Number of objects
	 * @param MinDistance Minimum distance between objects
	 * @return Array of transforms with random rotations
	 */
	static TArray<FTransform> GenerateScatteredPattern(
		const FVector& Center,
		float Radius,
		int32 Count,
		float MinDistance = 100.0f);

	/**
	 * Calculate bounding box for multiple actors
	 * @param Actors Actors to calculate bounds for
	 * @return Combined bounding box
	 */
	static FBox CalculateCombinedBounds(const TArray<AActor*>& Actors);

	/**
	 * Get center point of actors
	 * @param Actors Actors to get center of
	 * @return Center point
	 */
	static FVector GetActorsCenter(const TArray<AActor*>& Actors);

private:
	// Relationship keyword detection
	static ESpatialRelationship DetectRelationshipType(const FString& Query);

	// Direction parsing ("from the north", "facing east")
	static FVector ParseDirection(const FString& Query);

	// Distance parsing ("50 units away", "close to")
	static float ParseDistance(const FString& Query);
};
