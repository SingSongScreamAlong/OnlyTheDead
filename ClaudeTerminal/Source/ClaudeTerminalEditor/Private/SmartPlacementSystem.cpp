// Copyright Epic Games, Inc. All Rights Reserved.

#include "SmartPlacementSystem.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h"

FSmartPlacementResult FSmartPlacementSystem::FindOptimalPlacement(
	const FVector& DesiredLocation,
	const FRotator& DesiredRotation,
	AActor* ActorToPlace,
	UWorld* World,
	const FSmartPlacementOptions& Options)
{
	FSmartPlacementResult Result;
	Result.AdjustedLocation = DesiredLocation;
	Result.AdjustedRotation = DesiredRotation;

	if (!World || !ActorToPlace)
	{
		Result.bSuccess = false;
		Result.FailureReason = TEXT("Invalid world or actor");
		return Result;
	}

	// Get actor bounds
	FBox ActorBounds = ActorToPlace->GetComponentsBoundingBox(true);
	FVector BoundsExtent = ActorBounds.GetExtent();

	// Step 1: Snap to surface if enabled
	FVector SurfaceNormal = FVector::UpVector;
	if (Options.bSnapToSurface)
	{
		FVector SnappedLocation = SnapToSurface(DesiredLocation, World, Options.SnapDistance, SurfaceNormal);
		if (!SnappedLocation.Equals(DesiredLocation, 1.0f))
		{
			Result.AdjustedLocation = SnappedLocation;
			Result.bLocationAdjusted = true;
		}
	}

	// Step 2: Check slope angle
	float SlopeAngle = CalculateSlopeAngle(Result.AdjustedLocation, World);
	if (SlopeAngle > Options.MaxSlopeAngle)
	{
		Result.bSuccess = false;
		Result.FailureReason = FString::Printf(TEXT("Slope too steep: %.1f degrees (max: %.1f)"),
			SlopeAngle, Options.MaxSlopeAngle);
		return Result;
	}

	// Step 3: Align with surface if enabled
	if (Options.bAlignWithSurface && !SurfaceNormal.Equals(FVector::UpVector))
	{
		FRotator AlignedRotation = AlignWithSurface(DesiredRotation, SurfaceNormal);
		if (!AlignedRotation.Equals(DesiredRotation))
		{
			Result.AdjustedRotation = AlignedRotation;
			Result.bRotationAdjusted = true;
		}
	}

	// Step 4: Check for collisions if enabled
	if (Options.bAvoidCollisions)
	{
		if (CheckCollision(Result.AdjustedLocation, ActorBounds, World))
		{
			// Try to find nearby valid location
			FVector ValidLocation = FindNearestValidLocation(
				Result.AdjustedLocation,
				ActorBounds,
				World,
				Options.MinimumClearance * 2.0f,
				Options);

			if (!ValidLocation.Equals(Result.AdjustedLocation, 1.0f))
			{
				Result.AdjustedLocation = ValidLocation;
				Result.bLocationAdjusted = true;
			}
			else
			{
				Result.bSuccess = false;
				Result.FailureReason = TEXT("Collision detected and no valid location found nearby");
				return Result;
			}
		}
	}

	// Step 5: Physics-based placement if enabled
	if (Options.bUsePhysicsPlacement)
	{
		// This would simulate dropping the object and letting it settle
		// For now, we'll just ensure it's on a surface
		FVector PhysicsLocation = SnapToSurface(Result.AdjustedLocation, World, 1000.0f, SurfaceNormal);
		Result.AdjustedLocation = PhysicsLocation;
		Result.bLocationAdjusted = true;
	}

	Result.bSuccess = true;
	return Result;
}

bool FSmartPlacementSystem::IsLocationValid(
	const FVector& Location,
	const FBox& ActorBounds,
	UWorld* World,
	const FSmartPlacementOptions& Options)
{
	if (!World)
		return false;

	// Check slope
	float SlopeAngle = CalculateSlopeAngle(Location, World);
	if (SlopeAngle > Options.MaxSlopeAngle)
		return false;

	// Check collision
	if (Options.bAvoidCollisions && CheckCollision(Location, ActorBounds, World))
		return false;

	return true;
}

FVector FSmartPlacementSystem::SnapToSurface(
	const FVector& Location,
	UWorld* World,
	float SnapDistance,
	FVector& OutSurfaceNormal)
{
	if (!World)
	{
		OutSurfaceNormal = FVector::UpVector;
		return Location;
	}

	// Trace downward
	FVector TraceStart = Location + FVector(0, 0, SnapDistance);
	FVector TraceEnd = Location - FVector(0, 0, SnapDistance);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;

	if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
	{
		OutSurfaceNormal = HitResult.Normal;
		return HitResult.Location;
	}

	// No surface found, try upward trace
	TraceStart = Location - FVector(0, 0, SnapDistance);
	TraceEnd = Location + FVector(0, 0, SnapDistance);

	if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
	{
		OutSurfaceNormal = HitResult.Normal;
		return HitResult.Location;
	}

	OutSurfaceNormal = FVector::UpVector;
	return Location;
}

FRotator FSmartPlacementSystem::AlignWithSurface(const FRotator& BaseRotation, const FVector& SurfaceNormal)
{
	// Calculate rotation that aligns object's up vector with surface normal
	FVector UpVector = FVector::UpVector;
	FQuat AlignmentQuat = FQuat::FindBetweenNormals(UpVector, SurfaceNormal);
	FQuat BaseQuat = BaseRotation.Quaternion();
	FQuat FinalQuat = AlignmentQuat * BaseQuat;

	return FinalQuat.Rotator();
}

bool FSmartPlacementSystem::CheckCollision(const FVector& Location, const FBox& ActorBounds, UWorld* World)
{
	if (!World)
		return false;

	FVector BoundsExtent = ActorBounds.GetExtent();

	// Perform box overlap test
	FCollisionShape BoxShape = FCollisionShape::MakeBox(BoundsExtent);
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;

	TArray<FOverlapResult> Overlaps;
	bool bHasOverlap = World->OverlapMultiByChannel(
		Overlaps,
		Location,
		FQuat::Identity,
		ECC_WorldStatic,
		BoxShape,
		QueryParams);

	return bHasOverlap && Overlaps.Num() > 0;
}

FVector FSmartPlacementSystem::FindNearestValidLocation(
	const FVector& StartLocation,
	const FBox& ActorBounds,
	UWorld* World,
	float SearchRadius,
	const FSmartPlacementOptions& Options)
{
	if (!World)
		return StartLocation;

	// Try concentric circles at increasing distances
	const int32 NumAngles = 8;
	const float AngleStep = 360.0f / NumAngles;
	const int32 NumRadii = 5;
	const float RadiusStep = SearchRadius / NumRadii;

	for (int32 r = 1; r <= NumRadii; r++)
	{
		float CurrentRadius = r * RadiusStep;

		for (int32 a = 0; a < NumAngles; a++)
		{
			float Angle = a * AngleStep;
			FVector Offset = FVector(
				FMath::Cos(FMath::DegreesToRadians(Angle)) * CurrentRadius,
				FMath::Sin(FMath::DegreesToRadians(Angle)) * CurrentRadius,
				0.0f);

			FVector TestLocation = StartLocation + Offset;

			// Snap to surface at test location
			FVector SurfaceNormal;
			TestLocation = SnapToSurface(TestLocation, World, 200.0f, SurfaceNormal);

			if (IsLocationValid(TestLocation, ActorBounds, World, Options))
			{
				return TestLocation;
			}
		}
	}

	// No valid location found
	return StartLocation;
}

float FSmartPlacementSystem::CalculateSlopeAngle(const FVector& Location, UWorld* World)
{
	if (!World)
		return 0.0f;

	FVector SurfaceNormal;
	FVector SnappedLocation = SnapToSurface(Location, World, 500.0f, SurfaceNormal);

	// Calculate angle between surface normal and up vector
	float DotProduct = FVector::DotProduct(SurfaceNormal, FVector::UpVector);
	float AngleRadians = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f));
	float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	return AngleDegrees;
}
