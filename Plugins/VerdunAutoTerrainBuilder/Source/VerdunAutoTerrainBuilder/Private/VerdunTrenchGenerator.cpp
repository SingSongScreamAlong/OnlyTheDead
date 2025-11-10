// Copyright Epic Games, Inc. All Rights Reserved.

#include "VerdunTrenchGenerator.h"
#include "VerdunTerrainParameters.h"
#include "VerdunLocationData.h"
#include "Components/SplineComponent.h"
#include "Math/RandomStream.h"

AVerdunTrenchGenerator::AVerdunTrenchGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootSceneComponent;
}

void AVerdunTrenchGenerator::BeginPlay()
{
	Super::BeginPlay();
}

void AVerdunTrenchGenerator::GenerateTrenchNetworks()
{
	if (!TerrainParameters || !LocationDatabase)
	{
		UE_LOG(LogTemp, Error, TEXT("TerrainParameters or LocationDatabase not set. Cannot generate trenches."));
		return;
	}

	ClearTrenches();

	FRandomStream RandomStream(RandomSeed);

	// Generate trenches around major fortifications
	TArray<FVerdunLocation> Forts = LocationDatabase->GetFortifications();

	for (const FVerdunLocation& Fort : Forts)
	{
		// Generate concentric trench rings around major forts
		if (Fort.Name.Contains(TEXT("Douaumont")) ||
			Fort.Name.Contains(TEXT("Vaux")) ||
			Fort.Name.Contains(TEXT("Souville")))
		{
			GenerateFortTrenches(Fort.UE5Location, 500.0f, 3); // 500m radius, 3 rings
			GenerateCommunicationTrenches(Fort.UE5Location, Fort.UE5Location + FVector(0.0f, -100000.0f, 0.0f)); // To rear
		}
	}

	// Generate front-line trenches
	// French defensive lines (R1 and R2)
	bool bFoundR1 = false;
	FVerdunLocation R1Location;
	LocationDatabase->GetLocationByName(TEXT("R1 Trench"), bFoundR1);

	if (bFoundR1)
	{
		// Generate a curved defensive line
		FVector LineStart = R1Location.UE5Location + FVector(-200000.0f, 0.0f, 0.0f); // 2km west
		FVector LineEnd = R1Location.UE5Location + FVector(200000.0f, 0.0f, 0.0f); // 2km east
		GenerateFrontLineTrench(LineStart, LineEnd, EVerdunTrenchType::FrontLine);
	}

	// Generate German front lines
	FVector GermanLineCenter(0.0f, 200000.0f, 0.0f); // 2km north of battlefield center
	FVector GermanLineStart = GermanLineCenter + FVector(-300000.0f, 0.0f, 0.0f);
	FVector GermanLineEnd = GermanLineCenter + FVector(300000.0f, 0.0f, 0.0f);
	GenerateFrontLineTrench(GermanLineStart, GermanLineEnd, EVerdunTrenchType::FrontLine);

	UE_LOG(LogTemp, Log, TEXT("Generated %d trench segments"), GeneratedTrenches.Num());
}

void AVerdunTrenchGenerator::GenerateFortTrenches(FVector FortLocation, float RadiusMeters, int32 NumRings)
{
	FRandomStream RandomStream(RandomSeed + FortLocation.X);

	float RadiusCm = RadiusMeters * 100.0f;

	for (int32 Ring = 0; Ring < NumRings; Ring++)
	{
		float CurrentRadius = RadiusCm + (Ring * 20000.0f); // 200m spacing between rings

		// Generate circular trench with zigzag pattern
		const int32 NumSegments = 16; // Divide circle into 16 segments
		const float AngleStep = 360.0f / NumSegments;

		for (int32 Segment = 0; Segment < NumSegments; Segment++)
		{
			float StartAngle = FMath::DegreesToRadians(Segment * AngleStep);
			float EndAngle = FMath::DegreesToRadians((Segment + 1) * AngleStep);

			FVector StartPoint = FortLocation + FVector(
				FMath::Cos(StartAngle) * CurrentRadius,
				FMath::Sin(StartAngle) * CurrentRadius,
				0.0f
			);

			FVector EndPoint = FortLocation + FVector(
				FMath::Cos(EndAngle) * CurrentRadius,
				FMath::Sin(EndAngle) * CurrentRadius,
				0.0f
			);

			FTrenchSegment Segment;
			Segment.TrenchType = (Ring == 0) ? EVerdunTrenchType::FrontLine : EVerdunTrenchType::Support;
			Segment.WidthCm = TerrainParameters->TrenchParameters.WidthMeters * 100.0f;
			Segment.DepthCm = TerrainParameters->TrenchParameters.DepthMeters * 100.0f;
			Segment.SplinePoints = GenerateZigzagPattern(StartPoint, EndPoint, TraverseSpacingCm, TraverseDepthCm, RandomStream);
			Segment.bCollapsed = RandomStream.FRandRange(0.0f, 100.0f) < TrenchCollapsePercentage;

			GeneratedTrenches.Add(Segment);
		}
	}
}

void AVerdunTrenchGenerator::GenerateFrontLineTrench(FVector StartPoint, FVector EndPoint, EVerdunTrenchType TrenchType)
{
	FRandomStream RandomStream(RandomSeed + StartPoint.X + StartPoint.Y);

	FTrenchSegment Trench;
	Trench.TrenchType = TrenchType;
	Trench.WidthCm = TerrainParameters ? TerrainParameters->TrenchParameters.WidthMeters * 100.0f : 150.0f;
	Trench.DepthCm = TerrainParameters ? TerrainParameters->TrenchParameters.DepthMeters * 100.0f : 200.0f;
	Trench.SplinePoints = GenerateZigzagPattern(StartPoint, EndPoint, TraverseSpacingCm, TraverseDepthCm, RandomStream);
	Trench.bCollapsed = RandomStream.FRandRange(0.0f, 100.0f) < TrenchCollapsePercentage;

	GeneratedTrenches.Add(Trench);
}

void AVerdunTrenchGenerator::GenerateCommunicationTrenches(FVector FrontLineCenter, FVector RearPosition)
{
	FRandomStream RandomStream(RandomSeed + RearPosition.X);

	// Generate 3-4 communication trenches radiating from front to rear
	const int32 NumCommTrenches = 3;

	for (int32 i = 0; i < NumCommTrenches; i++)
	{
		// Offset each communication trench
		float OffsetAngle = FMath::DegreesToRadians((i - 1) * 30.0f); // -30, 0, +30 degrees
		FVector Offset(FMath::Cos(OffsetAngle) * 50000.0f, FMath::Sin(OffsetAngle) * 50000.0f, 0.0f);

		FVector Start = FrontLineCenter + Offset;
		FVector End = RearPosition + Offset;

		FTrenchSegment CommTrench;
		CommTrench.TrenchType = EVerdunTrenchType::Communication;
		CommTrench.WidthCm = TerrainParameters ? TerrainParameters->TrenchParameters.WidthMeters * 100.0f : 150.0f;
		CommTrench.DepthCm = TerrainParameters ? TerrainParameters->TrenchParameters.DepthMeters * 100.0f : 200.0f;

		// Communication trenches use less zigzag for faster movement
		CommTrench.SplinePoints = GenerateZigzagPattern(Start, End, TraverseSpacingCm * 2.0f, TraverseDepthCm * 0.5f, RandomStream);
		CommTrench.bCollapsed = RandomStream.FRandRange(0.0f, 100.0f) < (TrenchCollapsePercentage * 0.7f); // Less collapse

		GeneratedTrenches.Add(CommTrench);
	}
}

void AVerdunTrenchGenerator::ClearTrenches()
{
	GeneratedTrenches.Empty();

	// Clear spline components
	for (USplineComponent* Spline : TrenchSplines)
	{
		if (Spline)
		{
			Spline->DestroyComponent();
		}
	}
	TrenchSplines.Empty();

	UE_LOG(LogTemp, Log, TEXT("Cleared all trenches"));
}

void AVerdunTrenchGenerator::CreateSplineVisualization()
{
	// Clear existing splines
	for (USplineComponent* Spline : TrenchSplines)
	{
		if (Spline)
		{
			Spline->DestroyComponent();
		}
	}
	TrenchSplines.Empty();

	// Create spline component for each trench segment
	for (int32 i = 0; i < GeneratedTrenches.Num(); i++)
	{
		const FTrenchSegment& Trench = GeneratedTrenches[i];

		FString SplineName = FString::Printf(TEXT("TrenchSpline_%d"), i);
		USplineComponent* SplineComponent = NewObject<USplineComponent>(this, USplineComponent::StaticClass(), *SplineName);

		if (SplineComponent)
		{
			SplineComponent->SetupAttachment(RootComponent);
			SplineComponent->RegisterComponent();

			// Clear default points
			SplineComponent->ClearSplinePoints();

			// Add points from trench segment
			for (int32 PointIndex = 0; PointIndex < Trench.SplinePoints.Num(); PointIndex++)
			{
				SplineComponent->AddSplinePoint(Trench.SplinePoints[PointIndex], ESplineCoordinateSpace::World);
			}

			// Update spline
			SplineComponent->UpdateSpline();

			TrenchSplines.Add(SplineComponent);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Created %d spline visualizations"), TrenchSplines.Num());
}

void AVerdunTrenchGenerator::ApplyTrenchesToLandscape(ALandscape* Landscape)
{
	if (!Landscape)
	{
		UE_LOG(LogTemp, Error, TEXT("Landscape not set. Cannot apply trenches."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("ApplyTrenchesToLandscape: Direct landscape modification requires additional implementation."));
	UE_LOG(LogTemp, Log, TEXT("Generated %d trench segments ready for application"), GeneratedTrenches.Num());

	// TODO: Implement landscape excavation along trench splines
	// This would involve:
	// 1. Getting landscape edit interface
	// 2. For each trench spline, sample points along the path
	// 3. Lower terrain height along the trench path
	// 4. Adjust terrain to create trench walls
}

TArray<FVector> AVerdunTrenchGenerator::GenerateZigzagPattern(FVector Start, FVector End, float TraverseSpacing, float TraverseDepth, FRandomStream& RandomStream)
{
	TArray<FVector> Points;

	FVector Direction = (End - Start).GetSafeNormal();
	FVector Perpendicular = FVector::CrossProduct(Direction, FVector::UpVector);

	float TotalDistance = FVector::Dist(Start, End);
	int32 NumTraverses = FMath::Max(1, FMath::FloorToInt(TotalDistance / TraverseSpacing));

	Points.Add(Start);

	bool bLeftSide = true;
	for (int32 i = 0; i < NumTraverses; i++)
	{
		float T = (float)(i + 1) / (float)NumTraverses;
		FVector BasePoint = FMath::Lerp(Start, End, T);

		// Add zigzag offset
		float Offset = bLeftSide ? -TraverseDepth : TraverseDepth;
		FVector TraversePoint = BasePoint + (Perpendicular * Offset);

		Points.Add(TraversePoint);
		bLeftSide = !bLeftSide;
	}

	Points.Add(End);

	// Add random variation for realism
	AddRandomVariation(Points, TraverseDepth * 0.2f, RandomStream);

	return Points;
}

TArray<FVector> AVerdunTrenchGenerator::GenerateCurvedTrench(FVector Start, FVector End, float CurvatureAmount, FRandomStream& RandomStream)
{
	TArray<FVector> Points;

	const int32 NumPoints = 20;
	FVector Direction = (End - Start).GetSafeNormal();
	FVector Perpendicular = FVector::CrossProduct(Direction, FVector::UpVector);

	for (int32 i = 0; i <= NumPoints; i++)
	{
		float T = (float)i / (float)NumPoints;
		FVector BasePoint = FMath::Lerp(Start, End, T);

		// Add curve using sine wave
		float CurveOffset = FMath::Sin(T * PI) * CurvatureAmount;
		FVector CurvedPoint = BasePoint + (Perpendicular * CurveOffset);

		Points.Add(CurvedPoint);
	}

	AddRandomVariation(Points, CurvatureAmount * 0.1f, RandomStream);

	return Points;
}

void AVerdunTrenchGenerator::AddRandomVariation(TArray<FVector>& Points, float VariationAmount, FRandomStream& RandomStream)
{
	// Don't modify first and last points
	for (int32 i = 1; i < Points.Num() - 1; i++)
	{
		FVector Offset(
			RandomStream.FRandRange(-VariationAmount, VariationAmount),
			RandomStream.FRandRange(-VariationAmount, VariationAmount),
			0.0f
		);

		Points[i] += Offset;
	}
}
