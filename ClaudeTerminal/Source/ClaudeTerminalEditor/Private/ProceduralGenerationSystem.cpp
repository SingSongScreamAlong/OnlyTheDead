// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProceduralGenerationSystem.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

bool FProceduralGenerationSystem::GenerateTrenchNetwork(
	const FVector& StartLocation,
	const FVector& AreaSize,
	UWorld* World,
	const FGenerationConstraint& Constraints,
	FGenerationResult& OutResult)
{
	if (!World)
	{
		OutResult.ErrorMessage = TEXT("World is null");
		return false;
	}

	double StartTime = FPlatformTime::Seconds();

	UE_LOG(LogTemp, Log, TEXT("ProceduralGeneration: Generating trench network in %.0f x %.0f area"),
		AreaSize.X, AreaSize.Y);

	// Generate trench path using noise for realistic zigzag pattern
	FNoiseParameters NoiseParams;
	NoiseParams.Frequency = 0.01f;
	NoiseParams.Amplitude = 200.0f;
	NoiseParams.Seed = FMath::Rand();

	int32 TrenchSegments = FMath::FloorToInt(AreaSize.X / 500.0f);
	FVector CurrentPos = StartLocation;

	for (int32 i = 0; i < TrenchSegments; i++)
	{
		// Calculate next position with noise-based offset
		FVector2D NoiseInput(CurrentPos.X, CurrentPos.Y);
		float NoiseValue = GeneratePerlinNoise(NoiseInput, NoiseParams);

		FVector NextPos = CurrentPos + FVector(500, NoiseValue * 200, 0);

		// Check constraints
		if (CheckConstraints(NextPos, World, Constraints))
		{
			// In full implementation, would spawn trench mesh actors here
			// For now, just count
			OutResult.TotalGenerated++;
		}

		CurrentPos = NextPos;
	}

	OutResult.bSuccess = OutResult.TotalGenerated > 0;
	OutResult.GenerationTime = FPlatformTime::Seconds() - StartTime;

	UE_LOG(LogTemp, Log, TEXT("ProceduralGeneration: Generated %d trench segments in %.2f seconds"),
		OutResult.TotalGenerated, OutResult.GenerationTime);

	return OutResult.bSuccess;
}

bool FProceduralGenerationSystem::GenerateDestroyedVillage(
	const FVector& CenterLocation,
	float Radius,
	float DamageLevel,
	UWorld* World,
	const FGenerationConstraint& Constraints,
	FGenerationResult& OutResult)
{
	if (!World)
	{
		OutResult.ErrorMessage = TEXT("World is null");
		return false;
	}

	double StartTime = FPlatformTime::Seconds();

	UE_LOG(LogTemp, Log, TEXT("ProceduralGeneration: Generating destroyed village (damage: %.1f%%)"),
		DamageLevel * 100.0f);

	// Use Poisson disc for realistic building distribution
	FBox2D Bounds(FVector2D(-Radius, -Radius), FVector2D(Radius, Radius));
	TArray<FVector> BuildingPositions = GeneratePoissonDiscDistribution(Bounds, 400.0f);

	for (const FVector& Position : BuildingPositions)
	{
		FVector WorldPos = CenterLocation + FVector(Position.X, Position.Y, 0);

		if (!CheckConstraints(WorldPos, World, Constraints))
			continue;

		// Determine if building is destroyed based on damage level
		bool bDestroyed = FMath::FRand() < DamageLevel;

		// In full implementation:
		// - Spawn building mesh (intact or destroyed variant)
		// - Add rubble if destroyed
		// - Place craters nearby
		// - Add burned vegetation

		OutResult.TotalGenerated++;
	}

	// Generate craters based on damage level
	int32 CraterCount = FMath::FloorToInt(BuildingPositions.Num() * DamageLevel * 2);
	for (int32 i = 0; i < CraterCount; i++)
	{
		FVector CraterPos = CenterLocation + FMath::RandPointInCircle(Radius);
		// Would spawn crater mesh here
		OutResult.TotalGenerated++;
	}

	OutResult.bSuccess = OutResult.TotalGenerated > 0;
	OutResult.GenerationTime = FPlatformTime::Seconds() - StartTime;

	UE_LOG(LogTemp, Log, TEXT("ProceduralGeneration: Generated village with %d elements in %.2f seconds"),
		OutResult.TotalGenerated, OutResult.GenerationTime);

	return OutResult.bSuccess;
}

bool FProceduralGenerationSystem::GenerateForest(
	const FVector& CenterLocation,
	const FVector& AreaSize,
	float Density,
	UWorld* World,
	const FGenerationConstraint& Constraints,
	FGenerationResult& OutResult)
{
	if (!World)
	{
		OutResult.ErrorMessage = TEXT("World is null");
		return false;
	}

	double StartTime = FPlatformTime::Seconds();

	UE_LOG(LogTemp, Log, TEXT("ProceduralGeneration: Generating forest (density: %.1f)"), Density);

	// Calculate number of trees
	float Area = AreaSize.X * AreaSize.Y;
	int32 TreeCount = FMath::FloorToInt((Area / 1000.0f) * Density);

	// Use noise for natural clustering
	FNoiseParameters ClusterNoise;
	ClusterNoise.Frequency = 0.005f;
	ClusterNoise.Amplitude = 1.0f;
	ClusterNoise.Octaves = 3;

	// Use Poisson disc for spacing
	FBox2D Bounds(
		FVector2D(-AreaSize.X / 2, -AreaSize.Y / 2),
		FVector2D(AreaSize.X / 2, AreaSize.Y / 2));
	TArray<FVector> TreePositions = GeneratePoissonDiscDistribution(Bounds, 300.0f);

	for (const FVector& Position : TreePositions)
	{
		if (OutResult.TotalGenerated >= TreeCount)
			break;

		// Check noise for clustering
		FVector2D NoiseInput(Position.X, Position.Y);
		float ClusterValue = GeneratePerlinNoise(NoiseInput, ClusterNoise);

		// Only place tree if noise value exceeds threshold (creates clusters)
		if (ClusterValue < 0.3f)
			continue;

		FVector WorldPos = CenterLocation + FVector(Position.X, Position.Y, 0);

		if (!CheckConstraints(WorldPos, World, Constraints))
			continue;

		// In full implementation:
		// - Select tree species based on terrain/moisture
		// - Vary tree scale (0.8-1.5x)
		// - Rotate randomly
		// - Add undergrowth nearby

		OutResult.TotalGenerated++;
	}

	OutResult.bSuccess = OutResult.TotalGenerated > 0;
	OutResult.GenerationTime = FPlatformTime::Seconds() - StartTime;

	UE_LOG(LogTemp, Log, TEXT("ProceduralGeneration: Generated %d trees in %.2f seconds"),
		OutResult.TotalGenerated, OutResult.GenerationTime);

	return OutResult.bSuccess;
}

bool FProceduralGenerationSystem::GenerateBuildingCluster(
	const FVector& CenterLocation,
	int32 BuildingCount,
	const FString& Style,
	UWorld* World,
	const FGenerationConstraint& Constraints,
	FGenerationResult& OutResult)
{
	if (!World)
	{
		OutResult.ErrorMessage = TEXT("World is null");
		return false;
	}

	double StartTime = FPlatformTime::Seconds();

	UE_LOG(LogTemp, Log, TEXT("ProceduralGeneration: Generating %d buildings (%s style)"),
		BuildingCount, *Style);

	// Organic building placement
	for (int32 i = 0; i < BuildingCount; i++)
	{
		// Radial placement with some randomness
		float Angle = (i / (float)BuildingCount) * 360.0f + FMath::RandRange(-30.0f, 30.0f);
		float Radius = 500.0f + FMath::RandRange(-200.0f, 200.0f);

		FVector Offset(
			FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius,
			FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius,
			0);

		FVector BuildingPos = CenterLocation + Offset;

		if (!CheckConstraints(BuildingPos, World, Constraints))
			continue;

		// In full implementation:
		// - Select building mesh based on style
		// - Vary building size
		// - Orient toward center or along streets
		// - Add props around building

		OutResult.TotalGenerated++;
	}

	OutResult.bSuccess = OutResult.TotalGenerated > 0;
	OutResult.GenerationTime = FPlatformTime::Seconds() - StartTime;

	UE_LOG(LogTemp, Log, TEXT("ProceduralGeneration: Generated %d buildings in %.2f seconds"),
		OutResult.TotalGenerated, OutResult.GenerationTime);

	return OutResult.bSuccess;
}

bool FProceduralGenerationSystem::GenerateRoadNetwork(
	const FVector& StartPoint,
	const TArray<FVector>& EndPoints,
	float RoadWidth,
	UWorld* World,
	FGenerationResult& OutResult)
{
	if (!World)
	{
		OutResult.ErrorMessage = TEXT("World is null");
		return false;
	}

	double StartTime = FPlatformTime::Seconds();

	UE_LOG(LogTemp, Log, TEXT("ProceduralGeneration: Generating road network to %d destinations"),
		EndPoints.Num());

	// Generate roads from start to each endpoint
	for (const FVector& EndPoint : EndPoints)
	{
		FVector Direction = (EndPoint - StartPoint).GetSafeNormal();
		float Distance = FVector::Dist(StartPoint, EndPoint);
		int32 Segments = FMath::CeilToInt(Distance / 500.0f);

		FVector CurrentPos = StartPoint;

		for (int32 i = 0; i < Segments; i++)
		{
			float T = (i + 1) / (float)Segments;
			FVector NextPos = FMath::Lerp(StartPoint, EndPoint, T);

			// In full implementation:
			// - Spawn road spline mesh
			// - Follow terrain contours
			// - Add road markings
			// - Place streetlights

			CurrentPos = NextPos;
			OutResult.TotalGenerated++;
		}
	}

	OutResult.bSuccess = OutResult.TotalGenerated > 0;
	OutResult.GenerationTime = FPlatformTime::Seconds() - StartTime;

	UE_LOG(LogTemp, Log, TEXT("ProceduralGeneration: Generated %d road segments in %.2f seconds"),
		OutResult.TotalGenerated, OutResult.GenerationTime);

	return OutResult.bSuccess;
}

float FProceduralGenerationSystem::GeneratePerlinNoise(
	const FVector2D& Location,
	const FNoiseParameters& Params)
{
	float Total = 0.0f;
	float Frequency = Params.Frequency;
	float Amplitude = Params.Amplitude;

	for (int32 i = 0; i < Params.Octaves; i++)
	{
		Total += PerlinNoise2D(
			Location.X * Frequency,
			Location.Y * Frequency,
			Params.Seed + i) * Amplitude;

		Frequency *= Params.Lacunarity;
		Amplitude *= Params.Persistence;
	}

	return Total;
}

float FProceduralGenerationSystem::GenerateSimplexNoise(
	const FVector2D& Location,
	const FNoiseParameters& Params)
{
	// Simplified simplex noise implementation
	return SimplexNoise2D(Location.X * Params.Frequency, Location.Y * Params.Frequency) * Params.Amplitude;
}

TArray<FVector> FProceduralGenerationSystem::GeneratePoissonDiscDistribution(
	const FBox2D& Bounds,
	float MinDistance,
	int32 MaxAttempts)
{
	TArray<FVector> Points;
	TArray<FVector> ActiveList;

	// Start with random point
	FVector2D BoundsSize = Bounds.GetSize();
	FVector2D FirstPoint = Bounds.Min + FVector2D(
		FMath::FRand() * BoundsSize.X,
		FMath::FRand() * BoundsSize.Y);

	Points.Add(FVector(FirstPoint, 0));
	ActiveList.Add(FVector(FirstPoint, 0));

	while (ActiveList.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, ActiveList.Num() - 1);
		FVector Point = ActiveList[RandomIndex];

		bool bFoundValid = false;

		for (int32 i = 0; i < MaxAttempts; i++)
		{
			float Angle = FMath::FRand() * PI * 2.0f;
			float Distance = MinDistance * (1.0f + FMath::FRand());

			FVector2D NewPoint2D = FVector2D(Point.X, Point.Y) + FVector2D(
				FMath::Cos(Angle) * Distance,
				FMath::Sin(Angle) * Distance);

			// Check if in bounds
			if (!Bounds.IsInside(NewPoint2D))
				continue;

			// Check minimum distance from all points
			bool bTooClose = false;
			for (const FVector& ExistingPoint : Points)
			{
				float Dist = FVector2D::Distance(FVector2D(ExistingPoint.X, ExistingPoint.Y), NewPoint2D);
				if (Dist < MinDistance)
				{
					bTooClose = true;
					break;
				}
			}

			if (!bTooClose)
			{
				FVector NewPoint(NewPoint2D, 0);
				Points.Add(NewPoint);
				ActiveList.Add(NewPoint);
				bFoundValid = true;
				break;
			}
		}

		if (!bFoundValid)
		{
			ActiveList.RemoveAt(RandomIndex);
		}
	}

	UE_LOG(LogTemp, Verbose, TEXT("ProceduralGeneration: Poisson disc generated %d points"), Points.Num());

	return Points;
}

bool FProceduralGenerationSystem::CheckConstraints(
	const FVector& Location,
	UWorld* World,
	const FGenerationConstraint& Constraints)
{
	if (!World)
		return false;

	// Check slope
	float Slope = CalculateTerrainSlope(Location, World);
	if (Slope < Constraints.MinSlope || Slope > Constraints.MaxSlope)
		return false;

	// Check minimum distance from avoid actors
	for (AActor* AvoidActor : Constraints.AvoidActors)
	{
		if (AvoidActor)
		{
			float Distance = FVector::Dist(Location, AvoidActor->GetActorLocation());
			if (Distance < Constraints.MinSpacing)
				return false;
		}
	}

	return true;
}

float FProceduralGenerationSystem::CalculateTerrainSlope(const FVector& Location, UWorld* World)
{
	if (!World)
		return 0.0f;

	// Sample terrain at multiple points
	float SampleDistance = 100.0f;
	TArray<FVector> SamplePoints = {
		Location + FVector(SampleDistance, 0, 0),
		Location + FVector(-SampleDistance, 0, 0),
		Location + FVector(0, SampleDistance, 0),
		Location + FVector(0, -SampleDistance, 0)
	};

	float MaxHeightDiff = 0.0f;

	for (const FVector& SamplePoint : SamplePoints)
	{
		FHitResult Hit;
		FVector TraceStart = SamplePoint + FVector(0, 0, 10000);
		FVector TraceEnd = SamplePoint - FVector(0, 0, 10000);

		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
		{
			float HeightDiff = FMath::Abs(Hit.Location.Z - Location.Z);
			MaxHeightDiff = FMath::Max(MaxHeightDiff, HeightDiff);
		}
	}

	// Calculate slope angle
	float Angle = FMath::Atan(MaxHeightDiff / SampleDistance);
	return FMath::RadiansToDegrees(Angle);
}

TArray<FVector> FProceduralGenerationSystem::FindValidPlacements(
	const FBox& Bounds,
	int32 Count,
	UWorld* World,
	const FGenerationConstraint& Constraints)
{
	TArray<FVector> ValidLocations;

	// Use Poisson disc in 2D
	FBox2D Bounds2D(FVector2D(Bounds.Min.X, Bounds.Min.Y), FVector2D(Bounds.Max.X, Bounds.Max.Y));
	TArray<FVector> Candidates = GeneratePoissonDiscDistribution(Bounds2D, Constraints.MinSpacing);

	for (const FVector& Candidate : Candidates)
	{
		if (ValidLocations.Num() >= Count)
			break;

		FVector Location = Candidate;
		Location.Z = Bounds.Min.Z; // Would raycast to terrain in full implementation

		if (CheckConstraints(Location, World, Constraints))
		{
			ValidLocations.Add(Location);
		}
	}

	return ValidLocations;
}

// ===== NOISE IMPLEMENTATION =====

float FProceduralGenerationSystem::PerlinNoise2D(float x, float y, int32 seed)
{
	// Simplified Perlin noise
	int32 X = FMath::FloorToInt(x);
	int32 Y = FMath::FloorToInt(y);

	float xf = x - X;
	float yf = y - Y;

	float n00 = DotGridGradient(X, Y, x, y, seed);
	float n10 = DotGridGradient(X + 1, Y, x, y, seed);
	float n01 = DotGridGradient(X, Y + 1, x, y, seed);
	float n11 = DotGridGradient(X + 1, Y + 1, x, y, seed);

	float u = Fade(xf);
	float v = Fade(yf);

	float nx0 = Lerp(n00, n10, u);
	float nx1 = Lerp(n01, n11, u);

	return Lerp(nx0, nx1, v);
}

float FProceduralGenerationSystem::SimplexNoise2D(float x, float y)
{
	// Simplified simplex noise (placeholder)
	// Full implementation would use proper simplex algorithm
	return FMath::Sin(x * 0.1f) * FMath::Cos(y * 0.1f);
}

float FProceduralGenerationSystem::DotGridGradient(int32 ix, int32 iy, float x, float y, int32 seed)
{
	// Pseudo-random gradient
	int32 Hash = (ix * 374761393 + iy * 668265263 + seed) % 4;

	float dx = x - ix;
	float dy = y - iy;

	switch (Hash)
	{
	case 0: return dx + dy;
	case 1: return -dx + dy;
	case 2: return dx - dy;
	case 3: return -dx - dy;
	default: return 0;
	}
}

float FProceduralGenerationSystem::Fade(float t)
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}

float FProceduralGenerationSystem::Lerp(float a, float b, float t)
{
	return a + t * (b - a);
}
