// Copyright Epic Games, Inc. All Rights Reserved.

#include "RelationshipUnderstandingSystem.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

FRelationshipContext FRelationshipUnderstandingSystem::ParseRelationship(
	const FString& Query,
	const TArray<AActor*>& ContextActors)
{
	FRelationshipContext Context;
	Context.RawQuery = Query;
	Context.ReferenceActors = ContextActors;

	// Detect relationship type
	Context.Relationship = DetectRelationshipType(Query);

	// Parse direction if applicable
	Context.Direction = ParseDirection(Query);

	// Parse distance
	Context.Distance = ParseDistance(Query);

	// Check for exclusion keywords
	Context.bExcludeZone = Query.Contains(TEXT("but not")) || Query.Contains(TEXT("exclude"));

	return Context;
}

FRelationshipPlacementResult FRelationshipUnderstandingSystem::CalculatePlacement(
	const FRelationshipContext& Context,
	int32 Count,
	UWorld* World)
{
	FRelationshipPlacementResult Result;

	if (Count <= 0)
	{
		Result.ErrorMessage = TEXT("Invalid count");
		return Result;
	}

	TArray<FTransform> Transforms;

	switch (Context.Relationship)
	{
	case ESpatialRelationship::Around:
	{
		FVector Center = GetActorsCenter(Context.ReferenceActors);
		Transforms = GenerateCircularPattern(Center, Context.Distance, Count, true);
		break;
	}

	case ESpatialRelationship::Between:
	{
		if (Context.ReferencePoints.Num() >= 2)
		{
			Transforms = GeneratePathPattern(
				Context.ReferencePoints[0],
				Context.ReferencePoints[1],
				Count,
				false);
		}
		else if (Context.ReferenceActors.Num() >= 2)
		{
			FVector Start = Context.ReferenceActors[0]->GetActorLocation();
			FVector End = Context.ReferenceActors[1]->GetActorLocation();
			Transforms = GeneratePathPattern(Start, End, Count, false);
		}
		break;
	}

	case ESpatialRelationship::DefendingFrom:
	{
		FVector DefensePoint = GetActorsCenter(Context.ReferenceActors);
		Transforms = GenerateDefensiveArc(DefensePoint, Context.Direction, Count, 180.0f);
		break;
	}

	case ESpatialRelationship::Connecting:
	{
		if (Context.ReferencePoints.Num() >= 2)
		{
			Transforms = GeneratePathPattern(
				Context.ReferencePoints[0],
				Context.ReferencePoints[1],
				Count,
				true);
		}
		break;
	}

	case ESpatialRelationship::Scattered:
	{
		FVector Center = GetActorsCenter(Context.ReferenceActors);
		Transforms = GenerateScatteredPattern(Center, Context.Distance, Count, 100.0f);
		break;
	}

	case ESpatialRelationship::NextTo:
	{
		FBox Bounds = CalculateCombinedBounds(Context.ReferenceActors);
		Transforms = GeneratePerimeterPattern(Bounds, Context.Distance, 100.0f);
		// Limit to requested count
		if (Transforms.Num() > Count)
		{
			Transforms.SetNum(Count);
		}
		break;
	}

	case ESpatialRelationship::Facing:
	{
		FVector Center = GetActorsCenter(Context.ReferenceActors);
		FRotator FacingRotation = Context.Direction.Rotation();

		// Create line of objects facing the direction
		for (int32 i = 0; i < Count; i++)
		{
			FVector Offset = FVector(0, i * 200.0f, 0);
			FTransform Transform;
			Transform.SetLocation(Center + Offset);
			Transform.SetRotation(FacingRotation.Quaternion());
			Transforms.Add(Transform);
		}
		break;
	}

	default:
	{
		Result.ErrorMessage = TEXT("Unsupported relationship type");
		return Result;
	}
	}

	// Extract locations and rotations
	for (const FTransform& Transform : Transforms)
	{
		Result.PlacementLocations.Add(Transform.GetLocation());
		Result.PlacementRotations.Add(Transform.Rotator());
	}

	Result.bSuccess = Result.PlacementLocations.Num() > 0;
	return Result;
}

TArray<FTransform> FRelationshipUnderstandingSystem::GenerateCircularPattern(
	const FVector& Center,
	float Radius,
	int32 Count,
	bool bFaceCenter)
{
	TArray<FTransform> Transforms;

	float AngleStep = 360.0f / Count;

	for (int32 i = 0; i < Count; i++)
	{
		float Angle = i * AngleStep;
		float AngleRadians = FMath::DegreesToRadians(Angle);

		FVector Offset(
			FMath::Cos(AngleRadians) * Radius,
			FMath::Sin(AngleRadians) * Radius,
			0.0f);

		FVector Location = Center + Offset;

		FRotator Rotation = FRotator::ZeroRotator;
		if (bFaceCenter)
		{
			FVector ToCenter = Center - Location;
			Rotation = ToCenter.Rotation();
		}

		FTransform Transform;
		Transform.SetLocation(Location);
		Transform.SetRotation(Rotation.Quaternion());
		Transforms.Add(Transform);
	}

	return Transforms;
}

TArray<FTransform> FRelationshipUnderstandingSystem::GeneratePerimeterPattern(
	const FBox& Bounds,
	float Spacing,
	float Offset)
{
	TArray<FTransform> Transforms;

	FVector Center = Bounds.GetCenter();
	FVector Extent = Bounds.GetExtent();

	// Expand bounds by offset
	FVector ExpandedMin = Bounds.Min - FVector(Offset);
	FVector ExpandedMax = Bounds.Max + FVector(Offset);

	// Calculate perimeter length
	float Width = (ExpandedMax.X - ExpandedMin.X);
	float Depth = (ExpandedMax.Y - ExpandedMin.Y);
	float Perimeter = 2.0f * (Width + Depth);

	int32 NumPoints = FMath::CeilToInt(Perimeter / Spacing);

	float CurrentDistance = 0.0f;
	float SideLength;
	int32 Side = 0;

	for (int32 i = 0; i < NumPoints; i++)
	{
		FVector Location;
		FRotator Rotation = FRotator::ZeroRotator;

		// Determine which side we're on (0=front, 1=right, 2=back, 3=left)
		float T = (float)i / NumPoints;
		float PerimeterPosition = T * Perimeter;

		if (PerimeterPosition < Width)
		{
			// Front side (+X)
			float U = PerimeterPosition / Width;
			Location = FVector(
				ExpandedMax.X,
				ExpandedMin.Y + U * Depth,
				Center.Z);
			Rotation = FRotator(0, 180, 0); // Face inward
		}
		else if (PerimeterPosition < Width + Depth)
		{
			// Right side (+Y)
			float U = (PerimeterPosition - Width) / Depth;
			Location = FVector(
				ExpandedMax.X - U * Width,
				ExpandedMax.Y,
				Center.Z);
			Rotation = FRotator(0, -90, 0); // Face inward
		}
		else if (PerimeterPosition < 2.0f * Width + Depth)
		{
			// Back side (-X)
			float U = (PerimeterPosition - Width - Depth) / Width;
			Location = FVector(
				ExpandedMin.X,
				ExpandedMax.Y - U * Depth,
				Center.Z);
			Rotation = FRotator(0, 0, 0); // Face inward
		}
		else
		{
			// Left side (-Y)
			float U = (PerimeterPosition - 2.0f * Width - Depth) / Depth;
			Location = FVector(
				ExpandedMin.X + U * Width,
				ExpandedMin.Y,
				Center.Z);
			Rotation = FRotator(0, 90, 0); // Face inward
		}

		FTransform Transform;
		Transform.SetLocation(Location);
		Transform.SetRotation(Rotation.Quaternion());
		Transforms.Add(Transform);
	}

	return Transforms;
}

TArray<FTransform> FRelationshipUnderstandingSystem::GenerateDefensiveArc(
	const FVector& DefensePoint,
	const FVector& ThreatDirection,
	int32 Count,
	float ArcAngle)
{
	TArray<FTransform> Transforms;

	// Calculate base angle from threat direction
	FRotator ThreatRotation = ThreatDirection.Rotation();
	float BaseAngle = ThreatRotation.Yaw;

	// Calculate defensive arc positions
	float AngleStep = ArcAngle / (Count - 1);
	float StartAngle = BaseAngle - (ArcAngle / 2.0f);

	float DefenseRadius = 500.0f; // Distance from defense point

	for (int32 i = 0; i < Count; i++)
	{
		float Angle = StartAngle + (i * AngleStep);
		float AngleRadians = FMath::DegreesToRadians(Angle);

		// Calculate position in arc
		FVector Offset(
			FMath::Cos(AngleRadians) * DefenseRadius,
			FMath::Sin(AngleRadians) * DefenseRadius,
			0.0f);

		FVector Location = DefensePoint + Offset;

		// Face toward threat direction
		FRotator Rotation = (ThreatDirection * -1.0f).Rotation();

		FTransform Transform;
		Transform.SetLocation(Location);
		Transform.SetRotation(Rotation.Quaternion());
		Transforms.Add(Transform);
	}

	return Transforms;
}

TArray<FTransform> FRelationshipUnderstandingSystem::GeneratePathPattern(
	const FVector& StartPoint,
	const FVector& EndPoint,
	int32 Count,
	bool bAlignWithPath)
{
	TArray<FTransform> Transforms;

	FVector PathVector = EndPoint - StartPoint;
	FRotator PathRotation = PathVector.Rotation();

	for (int32 i = 0; i < Count; i++)
	{
		float T = (float)i / FMath::Max(1, Count - 1);
		FVector Location = FMath::Lerp(StartPoint, EndPoint, T);

		FRotator Rotation = FRotator::ZeroRotator;
		if (bAlignWithPath)
		{
			Rotation = PathRotation;
		}

		FTransform Transform;
		Transform.SetLocation(Location);
		Transform.SetRotation(Rotation.Quaternion());
		Transforms.Add(Transform);
	}

	return Transforms;
}

TArray<FTransform> FRelationshipUnderstandingSystem::GenerateScatteredPattern(
	const FVector& Center,
	float Radius,
	int32 Count,
	float MinDistance)
{
	TArray<FTransform> Transforms;
	TArray<FVector> PlacedLocations;

	int32 MaxAttempts = Count * 10;
	int32 Attempts = 0;

	while (Transforms.Num() < Count && Attempts < MaxAttempts)
	{
		Attempts++;

		// Generate random position
		FVector2D RandomCircle = FMath::RandPointInCircle(Radius);
		FVector TestLocation = Center + FVector(RandomCircle.X, RandomCircle.Y, 0.0f);

		// Check minimum distance from other placed objects
		bool bTooClose = false;
		for (const FVector& PlacedLoc : PlacedLocations)
		{
			if (FVector::Dist(TestLocation, PlacedLoc) < MinDistance)
			{
				bTooClose = true;
				break;
			}
		}

		if (!bTooClose)
		{
			PlacedLocations.Add(TestLocation);

			// Random rotation
			FRotator RandomRotation(0, FMath::RandRange(0.0f, 360.0f), 0);

			FTransform Transform;
			Transform.SetLocation(TestLocation);
			Transform.SetRotation(RandomRotation.Quaternion());
			Transforms.Add(Transform);
		}
	}

	return Transforms;
}

FBox FRelationshipUnderstandingSystem::CalculateCombinedBounds(const TArray<AActor*>& Actors)
{
	FBox CombinedBounds(ForceInit);

	for (AActor* Actor : Actors)
	{
		if (Actor)
		{
			FBox ActorBounds = Actor->GetComponentsBoundingBox(true);
			CombinedBounds += ActorBounds;
		}
	}

	return CombinedBounds;
}

FVector FRelationshipUnderstandingSystem::GetActorsCenter(const TArray<AActor*>& Actors)
{
	if (Actors.Num() == 0)
		return FVector::ZeroVector;

	FVector Sum = FVector::ZeroVector;
	for (AActor* Actor : Actors)
	{
		if (Actor)
		{
			Sum += Actor->GetActorLocation();
		}
	}

	return Sum / Actors.Num();
}

ESpatialRelationship FRelationshipUnderstandingSystem::DetectRelationshipType(const FString& Query)
{
	FString LowerQuery = Query.ToLower();

	if (LowerQuery.Contains(TEXT("around")) || LowerQuery.Contains(TEXT("surrounding")))
		return ESpatialRelationship::Around;

	if (LowerQuery.Contains(TEXT("between")))
		return ESpatialRelationship::Between;

	if (LowerQuery.Contains(TEXT("defending")) || LowerQuery.Contains(TEXT("defensive")))
		return ESpatialRelationship::DefendingFrom;

	if (LowerQuery.Contains(TEXT("connect")) || LowerQuery.Contains(TEXT("connecting")))
		return ESpatialRelationship::Connecting;

	if (LowerQuery.Contains(TEXT("scatter")) || LowerQuery.Contains(TEXT("random")))
		return ESpatialRelationship::Scattered;

	if (LowerQuery.Contains(TEXT("next to")) || LowerQuery.Contains(TEXT("beside")))
		return ESpatialRelationship::NextTo;

	if (LowerQuery.Contains(TEXT("facing")) || LowerQuery.Contains(TEXT("oriented")))
		return ESpatialRelationship::Facing;

	if (LowerQuery.Contains(TEXT("above")) || LowerQuery.Contains(TEXT("over")))
		return ESpatialRelationship::Above;

	if (LowerQuery.Contains(TEXT("below")) || LowerQuery.Contains(TEXT("under")))
		return ESpatialRelationship::Below;

	if (LowerQuery.Contains(TEXT("in front")))
		return ESpatialRelationship::InFrontOf;

	if (LowerQuery.Contains(TEXT("behind")))
		return ESpatialRelationship::Behind;

	return ESpatialRelationship::None;
}

FVector FRelationshipUnderstandingSystem::ParseDirection(const FString& Query)
{
	FString LowerQuery = Query.ToLower();

	if (LowerQuery.Contains(TEXT("north")))
		return FVector(1, 0, 0);
	if (LowerQuery.Contains(TEXT("south")))
		return FVector(-1, 0, 0);
	if (LowerQuery.Contains(TEXT("east")))
		return FVector(0, 1, 0);
	if (LowerQuery.Contains(TEXT("west")))
		return FVector(0, -1, 0);

	return FVector::ForwardVector;
}

float FRelationshipUnderstandingSystem::ParseDistance(const FString& Query)
{
	// Look for numeric values followed by "units" or distance keywords
	FString LowerQuery = Query.ToLower();

	// Simple distance keywords
	if (LowerQuery.Contains(TEXT("close")) || LowerQuery.Contains(TEXT("near")))
		return 200.0f;
	if (LowerQuery.Contains(TEXT("far")) || LowerQuery.Contains(TEXT("distant")))
		return 1000.0f;

	// Default distance
	return 500.0f;
}
