// Copyright Only The Dead Development Team. All Rights Reserved.

#include "TerrainSculptor.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeInfo.h"
#include "LandscapeEdit.h"
#include "LandscapeDataAccess.h"
#include "EngineUtils.h"

UTerrainSculptor::UTerrainSculptor()
{
}

ALandscape* UTerrainSculptor::FindLandscape(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<ALandscape> It(World); It; ++It)
	{
		return *It;
	}

	return nullptr;
}

bool UTerrainSculptor::SculptTerrain(
	UWorld* World,
	const FTerrainSculptParams& Params,
	FString& OutResult)
{
	switch (Params.Operation)
	{
	case ETerrainOperation::Raise:
		return RaiseTerrain(World, Params.Location, Params.Radius, Params.Strength, OutResult);

	case ETerrainOperation::Lower:
		return LowerTerrain(World, Params.Location, Params.Radius, Params.Strength, OutResult);

	case ETerrainOperation::Flatten:
		return FlattenTerrain(World, Params.Location, Params.Radius, Params.TargetHeight, OutResult);

	case ETerrainOperation::Smooth:
		return SmoothTerrain(World, Params.Location, Params.Radius, Params.Strength, OutResult);

	case ETerrainOperation::Noise:
		return AddTerrainNoise(World, Params.Location, Params.Radius, Params.NoiseScale, Params.Strength, OutResult);

	case ETerrainOperation::Terrace:
		return CreateTerraces(World, Params.Location, Params.Radius, Params.TerraceSteps, OutResult);

	default:
		OutResult = TEXT("Unknown terrain operation");
		return false;
	}
}

bool UTerrainSculptor::CreateCrater(
	UWorld* World,
	const FCraterParams& Params,
	FString& OutResult)
{
	ALandscape* Landscape = FindLandscape(World);
	if (!Landscape)
	{
		OutResult = TEXT("No landscape found in level");
		return false;
	}

	FVector2D Center(Params.Location.X, Params.Location.Y);
	float Radius = Params.Diameter / 2.0f;

	auto CraterFunction = [&](const FVector2D& Point, float Distance, float OriginalHeight) -> float
	{
		return CalculateCraterHeight(Point, Center, Params, OriginalHeight);
	};

	bool Success = ModifyLandscapeHeight(Landscape, Center, Radius * 1.5f, CraterFunction, OutResult);

	if (Success)
	{
		OutResult = FString::Printf(
			TEXT("Created artillery crater at %s (%.1fm diameter, %.1fm deep)"),
			*Params.Location.ToString(),
			Params.Diameter / 100.0f,
			Params.Depth / 100.0f
		);
	}

	return Success;
}

bool UTerrainSculptor::ExcavateTrench(
	UWorld* World,
	const FTrenchExcavationParams& Params,
	FString& OutResult)
{
	if (Params.Path.Num() < 2)
	{
		OutResult = TEXT("Trench path must have at least 2 points");
		return false;
	}

	ALandscape* Landscape = FindLandscape(World);
	if (!Landscape)
	{
		OutResult = TEXT("No landscape found in level");
		return false;
	}

	// Excavate each segment
	for (int32 i = 0; i < Params.Path.Num() - 1; i++)
	{
		FString SegmentResult;
		if (!ExcavateTrenchSegment(Landscape, Params.Path[i], Params.Path[i + 1], Params, SegmentResult))
		{
			OutResult = FString::Printf(TEXT("Failed at segment %d: %s"), i, *SegmentResult);
			return false;
		}
	}

	float TotalLength = 0.0f;
	for (int32 i = 0; i < Params.Path.Num() - 1; i++)
	{
		TotalLength += FVector::Dist(Params.Path[i], Params.Path[i + 1]);
	}

	OutResult = FString::Printf(
		TEXT("Excavated trench %.1fm long, %.1fm wide, %.1fm deep"),
		TotalLength / 100.0f,
		Params.Width / 100.0f,
		Params.Depth / 100.0f
	);

	return true;
}

bool UTerrainSculptor::RaiseTerrain(
	UWorld* World,
	const FVector& Location,
	float Radius,
	float Strength,
	FString& OutResult)
{
	ALandscape* Landscape = FindLandscape(World);
	if (!Landscape)
	{
		OutResult = TEXT("No landscape found");
		return false;
	}

	FVector2D Center(Location.X, Location.Y);

	auto RaiseFunction = [Strength](const FVector2D& Point, float Distance, float OriginalHeight) -> float
	{
		return OriginalHeight + (Strength * 100.0f); // Raise by Strength cm
	};

	bool Success = ModifyLandscapeHeight(Landscape, Center, Radius, RaiseFunction, OutResult);

	if (Success)
	{
		OutResult = FString::Printf(
			TEXT("Raised terrain at %s (radius %.1fm, strength %.2f)"),
			*Location.ToString(),
			Radius / 100.0f,
			Strength
		);
	}

	return Success;
}

bool UTerrainSculptor::LowerTerrain(
	UWorld* World,
	const FVector& Location,
	float Radius,
	float Strength,
	FString& OutResult)
{
	ALandscape* Landscape = FindLandscape(World);
	if (!Landscape)
	{
		OutResult = TEXT("No landscape found");
		return false;
	}

	FVector2D Center(Location.X, Location.Y);

	auto LowerFunction = [Strength](const FVector2D& Point, float Distance, float OriginalHeight) -> float
	{
		return OriginalHeight - (Strength * 100.0f); // Lower by Strength cm
	};

	bool Success = ModifyLandscapeHeight(Landscape, Center, Radius, LowerFunction, OutResult);

	if (Success)
	{
		OutResult = FString::Printf(
			TEXT("Lowered terrain at %s (radius %.1fm, strength %.2f)"),
			*Location.ToString(),
			Radius / 100.0f,
			Strength
		);
	}

	return Success;
}

bool UTerrainSculptor::FlattenTerrain(
	UWorld* World,
	const FVector& Location,
	float Radius,
	float TargetHeight,
	FString& OutResult)
{
	ALandscape* Landscape = FindLandscape(World);
	if (!Landscape)
	{
		OutResult = TEXT("No landscape found");
		return false;
	}

	FVector2D Center(Location.X, Location.Y);

	auto FlattenFunction = [TargetHeight, Radius](const FVector2D& Point, float Distance, float OriginalHeight) -> float
	{
		float T = CalculateFalloff(Distance, Radius, 0.5f);
		return FMath::Lerp(TargetHeight, OriginalHeight, T);
	};

	bool Success = ModifyLandscapeHeight(Landscape, Center, Radius, FlattenFunction, OutResult);

	if (Success)
	{
		OutResult = FString::Printf(
			TEXT("Flattened terrain at %s to height %.1f (radius %.1fm)"),
			*Location.ToString(),
			TargetHeight,
			Radius / 100.0f
		);
	}

	return Success;
}

bool UTerrainSculptor::SmoothTerrain(
	UWorld* World,
	const FVector& Location,
	float Radius,
	float Strength,
	FString& OutResult)
{
	ALandscape* Landscape = FindLandscape(World);
	if (!Landscape)
	{
		OutResult = TEXT("No landscape found");
		return false;
	}

	// Get landscape info
	ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
	if (!LandscapeInfo)
	{
		OutResult = TEXT("Failed to get landscape info");
		return false;
	}

	FVector2D Center(Location.X, Location.Y);

	// Sample surrounding heights for averaging
	FScopedSetLandscapeEditingLayer Scope(Landscape);

	FLandscapeEditDataInterface DataInterface(LandscapeInfo);
	DataInterface.SetShouldDirtyPackage(true);

	FIntPoint CenterCoords = WorldToLandscapeCoordinates(Landscape, Center);
	int32 RadiusInVerts = FMath::CeilToInt(Radius / Landscape->GetActorScale3D().X);

	// Read heights in radius
	TMap<FIntPoint, float> Heights;
	for (int32 Y = -RadiusInVerts; Y <= RadiusInVerts; Y++)
	{
		for (int32 X = -RadiusInVerts; X <= RadiusInVerts; X++)
		{
			FIntPoint Coords = CenterCoords + FIntPoint(X, Y);
			uint16 HeightValue;
			DataInterface.GetHeightData(Coords.X, Coords.Y, HeightValue);
			Heights.Add(Coords, (float)HeightValue);
		}
	}

	// Smooth by averaging
	for (int32 Y = -RadiusInVerts; Y <= RadiusInVerts; Y++)
	{
		for (int32 X = -RadiusInVerts; X <= RadiusInVerts; X++)
		{
			FIntPoint Coords = CenterCoords + FIntPoint(X, Y);
			FVector2D WorldPos = LandscapeToWorldCoordinates(Landscape, Coords);
			float Distance = FVector2D::Distance(WorldPos, Center);

			if (Distance > Radius)
			{
				continue;
			}

			// Average neighboring heights
			float Sum = 0.0f;
			int32 Count = 0;
			for (int32 NeighborY = -1; NeighborY <= 1; NeighborY++)
			{
				for (int32 NeighborX = -1; NeighborX <= 1; NeighborX++)
				{
					FIntPoint NeighborCoords = Coords + FIntPoint(NeighborX, NeighborY);
					if (Heights.Contains(NeighborCoords))
					{
						Sum += Heights[NeighborCoords];
						Count++;
					}
				}
			}

			float AverageHeight = (Count > 0) ? (Sum / Count) : Heights[Coords];
			float OriginalHeight = Heights[Coords];

			// Blend based on strength and falloff
			float Falloff = CalculateFalloff(Distance, Radius, 0.5f);
			float NewHeight = FMath::Lerp(AverageHeight, OriginalHeight, Falloff * (1.0f - Strength));

			DataInterface.SetHeightData(Coords.X, Coords.Y, (uint16)FMath::Clamp(NewHeight, 0.0f, 65535.0f));
		}
	}

	DataInterface.Flush();

	OutResult = FString::Printf(
		TEXT("Smoothed terrain at %s (radius %.1fm)"),
		*Location.ToString(),
		Radius / 100.0f
	);

	return true;
}

bool UTerrainSculptor::AddTerrainNoise(
	UWorld* World,
	const FVector& Location,
	float Radius,
	float Scale,
	float Strength,
	FString& OutResult)
{
	ALandscape* Landscape = FindLandscape(World);
	if (!Landscape)
	{
		OutResult = TEXT("No landscape found");
		return false;
	}

	FVector2D Center(Location.X, Location.Y);

	auto NoiseFunction = [Scale, Strength](const FVector2D& Point, float Distance, float OriginalHeight) -> float
	{
		// Simple Perlin-like noise
		float NoiseValue = FMath::PerlinNoise2D(Point / (Scale * 100.0f));
		return OriginalHeight + (NoiseValue * Strength * 50.0f);
	};

	bool Success = ModifyLandscapeHeight(Landscape, Center, Radius, NoiseFunction, OutResult);

	if (Success)
	{
		OutResult = FString::Printf(
			TEXT("Added noise to terrain at %s (radius %.1fm)"),
			*Location.ToString(),
			Radius / 100.0f
		);
	}

	return Success;
}

bool UTerrainSculptor::CreateTerraces(
	UWorld* World,
	const FVector& Location,
	float Radius,
	int32 Steps,
	FString& OutResult)
{
	ALandscape* Landscape = FindLandscape(World);
	if (!Landscape)
	{
		OutResult = TEXT("No landscape found");
		return false;
	}

	FVector2D Center(Location.X, Location.Y);

	auto TerraceFunction = [Steps, Radius](const FVector2D& Point, float Distance, float OriginalHeight) -> float
	{
		// Quantize height into steps
		float StepHeight = 50.0f; // 0.5m per step
		float Quantized = FMath::RoundToFloat(OriginalHeight / StepHeight) * StepHeight;

		float Falloff = CalculateFalloff(Distance, Radius, 0.5f);
		return FMath::Lerp(Quantized, OriginalHeight, Falloff);
	};

	bool Success = ModifyLandscapeHeight(Landscape, Center, Radius, TerraceFunction, OutResult);

	if (Success)
	{
		OutResult = FString::Printf(
			TEXT("Created terraces at %s (%d steps)"),
			*Location.ToString(),
			Steps
		);
	}

	return Success;
}

float UTerrainSculptor::GetTerrainHeight(UWorld* World, const FVector2D& Location)
{
	ALandscape* Landscape = FindLandscape(World);
	if (!Landscape)
	{
		return 0.0f;
	}

	ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
	if (!LandscapeInfo)
	{
		return 0.0f;
	}

	FLandscapeEditDataInterface DataInterface(LandscapeInfo);
	FIntPoint Coords = WorldToLandscapeCoordinates(Landscape, Location);

	uint16 HeightValue;
	DataInterface.GetHeightData(Coords.X, Coords.Y, HeightValue);

	return (float)HeightValue;
}

FVector UTerrainSculptor::GetTerrainNormal(UWorld* World, const FVector2D& Location)
{
	// Sample heights around the point to calculate normal
	float CenterHeight = GetTerrainHeight(World, Location);
	float RightHeight = GetTerrainHeight(World, Location + FVector2D(100, 0));
	float UpHeight = GetTerrainHeight(World, Location + FVector2D(0, 100));

	FVector Right(100, 0, RightHeight - CenterHeight);
	FVector Up(0, 100, UpHeight - CenterHeight);

	return FVector::CrossProduct(Right, Up).GetSafeNormal();
}

TArray<float> UTerrainSculptor::SampleHeightmap(
	UWorld* World,
	const FVector2D& Center,
	float Radius,
	int32 SampleCount)
{
	TArray<float> Samples;

	for (int32 i = 0; i < SampleCount; i++)
	{
		float Angle = (float)i / (float)SampleCount * 2.0f * PI;
		float Distance = FMath::FRandRange(0.0f, Radius);

		FVector2D SamplePos = Center + FVector2D(
			FMath::Cos(Angle) * Distance,
			FMath::Sin(Angle) * Distance
		);

		Samples.Add(GetTerrainHeight(World, SamplePos));
	}

	return Samples;
}

// ============================================================================
// PRIVATE IMPLEMENTATION
// ============================================================================

bool UTerrainSculptor::ModifyLandscapeHeight(
	ALandscape* Landscape,
	const FVector2D& Center,
	float Radius,
	TFunction<float(const FVector2D&, float, float)> HeightFunction,
	FString& OutResult)
{
	if (!Landscape)
	{
		OutResult = TEXT("No landscape provided");
		return false;
	}

	ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
	if (!LandscapeInfo)
	{
		OutResult = TEXT("Failed to get landscape info");
		return false;
	}

	FScopedSetLandscapeEditingLayer Scope(Landscape);

	FLandscapeEditDataInterface DataInterface(LandscapeInfo);
	DataInterface.SetShouldDirtyPackage(true);

	// Calculate affected region
	FIntPoint CenterCoords = WorldToLandscapeCoordinates(Landscape, Center);
	int32 RadiusInVerts = FMath::CeilToInt(Radius / Landscape->GetActorScale3D().X);

	int32 ModifiedCount = 0;

	for (int32 Y = -RadiusInVerts; Y <= RadiusInVerts; Y++)
	{
		for (int32 X = -RadiusInVerts; X <= RadiusInVerts; X++)
		{
			FIntPoint Coords = CenterCoords + FIntPoint(X, Y);
			FVector2D WorldPos = LandscapeToWorldCoordinates(Landscape, Coords);

			float Distance = FVector2D::Distance(WorldPos, Center);
			if (Distance > Radius)
			{
				continue;
			}

			// Get original height
			uint16 OriginalHeightValue;
			DataInterface.GetHeightData(Coords.X, Coords.Y, OriginalHeightValue);
			float OriginalHeight = (float)OriginalHeightValue;

			// Calculate new height
			float NewHeight = HeightFunction(WorldPos, Distance, OriginalHeight);
			NewHeight = FMath::Clamp(NewHeight, 0.0f, 65535.0f);

			// Set new height
			DataInterface.SetHeightData(Coords.X, Coords.Y, (uint16)NewHeight);
			ModifiedCount++;
		}
	}

	DataInterface.Flush();

	if (ModifiedCount == 0)
	{
		OutResult = TEXT("No vertices modified (check radius and location)");
		return false;
	}

	OutResult = FString::Printf(TEXT("Modified %d landscape vertices"), ModifiedCount);
	return true;
}

float UTerrainSculptor::CalculateFalloff(float Distance, float Radius, float FalloffAmount)
{
	if (Distance >= Radius)
	{
		return 1.0f;
	}

	float T = Distance / Radius;
	return SmoothFalloff(T * (1.0f + FalloffAmount));
}

float UTerrainSculptor::SmoothFalloff(float T)
{
	// Smooth interpolation (smoothstep)
	T = FMath::Clamp(T, 0.0f, 1.0f);
	return T * T * (3.0f - 2.0f * T);
}

float UTerrainSculptor::CalculateCraterHeight(
	const FVector2D& Point,
	const FVector2D& Center,
	const FCraterParams& Params,
	float OriginalHeight)
{
	float Distance = FVector2D::Distance(Point, Center);
	float Radius = Params.Diameter / 2.0f;

	if (Distance > Radius * 1.5f)
	{
		return OriginalHeight;
	}

	// Crater profile: depression in center, rim around edge
	float NormalizedDist = Distance / Radius;

	float CraterDepth = Params.Depth;
	float RimHeight = Params.RimHeight;

	float HeightDelta = 0.0f;

	if (NormalizedDist < 1.0f)
	{
		// Inside crater
		float DepthCurve = 1.0f - (NormalizedDist * NormalizedDist);
		HeightDelta = -CraterDepth * DepthCurve;

		// Add some randomness if requested
		if (Params.bRandomizeShape)
		{
			FRandomStream Random(Params.RandomSeed);
			float Noise = Random.FRandRange(-0.2f, 0.2f);
			HeightDelta *= (1.0f + Noise);
		}
	}
	else if (NormalizedDist < 1.5f)
	{
		// Rim area
		float RimT = (NormalizedDist - 1.0f) / 0.5f; // 0 at edge, 1 at outer rim
		float RimCurve = FMath::Sin((1.0f - RimT) * PI * 0.5f);
		HeightDelta = RimHeight * RimCurve;
	}

	return OriginalHeight + HeightDelta;
}

bool UTerrainSculptor::ExcavateTrenchSegment(
	ALandscape* Landscape,
	const FVector& Start,
	const FVector& End,
	const FTrenchExcavationParams& Params,
	FString& OutResult)
{
	FVector Direction = (End - Start).GetSafeNormal();
	FVector Right = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();
	float Length = FVector::Dist(Start, End);

	int32 Steps = FMath::CeilToInt(Length / 50.0f); // Sample every 50 cm

	for (int32 i = 0; i <= Steps; i++)
	{
		float T = (float)i / (float)Steps;
		FVector Position = FMath::Lerp(Start, End, T);

		// Excavate cross-section at this point
		float HalfWidth = Params.Width / 2.0f;

		for (float Offset = -HalfWidth; Offset <= HalfWidth; Offset += 10.0f)
		{
			FVector Point = Position + (Right * Offset);
			FVector2D Point2D(Point.X, Point.Y);

			// Calculate depth based on distance from center
			float DistFromCenter = FMath::Abs(Offset);
			float DepthAtPoint = 0.0f;

			if (DistFromCenter < Params.FloorWidth / 2.0f)
			{
				// Flat floor
				DepthAtPoint = Params.Depth;
			}
			else
			{
				// Sloped walls
				float WallT = (DistFromCenter - Params.FloorWidth / 2.0f) / (HalfWidth - Params.FloorWidth / 2.0f);
				DepthAtPoint = Params.Depth * (1.0f - WallT);
			}

			// Modify terrain
			auto TrenchFunction = [DepthAtPoint](const FVector2D& Pt, float Dist, float OrigHeight) -> float
			{
				return OrigHeight - DepthAtPoint;
			};

			FString TempResult;
			ModifyLandscapeHeight(Landscape, Point2D, 50.0f, TrenchFunction, TempResult);
		}

		// Create parapet if requested
		if (Params.bCreateParapet)
		{
			FVector ParapetPos = Position + (Right * (HalfWidth + 50.0f));
			FVector2D Parapet2D(ParapetPos.X, ParapetPos.Y);

			auto ParapetFunction = [Params](const FVector2D& Pt, float Dist, float OrigHeight) -> float
			{
				return OrigHeight + Params.ParapetHeight;
			};

			FString TempResult;
			ModifyLandscapeHeight(Landscape, Parapet2D, 100.0f, ParapetFunction, TempResult);
		}
	}

	OutResult = FString::Printf(TEXT("Excavated trench segment %.1fm"), Length / 100.0f);
	return true;
}

void UTerrainSculptor::GetLandscapeExtent(ALandscape* Landscape, FVector2D& Min, FVector2D& Max, float& ZScale)
{
	if (!Landscape)
	{
		return;
	}

	FIntRect Bounds = Landscape->GetBoundingRect();
	FVector Scale = Landscape->GetActorScale3D();

	Min = FVector2D(Bounds.Min.X * Scale.X, Bounds.Min.Y * Scale.Y);
	Max = FVector2D(Bounds.Max.X * Scale.X, Bounds.Max.Y * Scale.Y);
	ZScale = Scale.Z;
}

FIntPoint UTerrainSculptor::WorldToLandscapeCoordinates(ALandscape* Landscape, const FVector2D& WorldLocation)
{
	if (!Landscape)
	{
		return FIntPoint::ZeroValue;
	}

	FVector LandscapeSpaceLocation = Landscape->GetTransform().InverseTransformPosition(
		FVector(WorldLocation.X, WorldLocation.Y, 0.0f)
	);

	return FIntPoint(
		FMath::RoundToInt(LandscapeSpaceLocation.X),
		FMath::RoundToInt(LandscapeSpaceLocation.Y)
	);
}

FVector2D UTerrainSculptor::LandscapeToWorldCoordinates(ALandscape* Landscape, const FIntPoint& LandscapeCoords)
{
	if (!Landscape)
	{
		return FVector2D::ZeroVector;
	}

	FVector WorldLocation = Landscape->GetTransform().TransformPosition(
		FVector(LandscapeCoords.X, LandscapeCoords.Y, 0.0f)
	);

	return FVector2D(WorldLocation.X, WorldLocation.Y);
}
