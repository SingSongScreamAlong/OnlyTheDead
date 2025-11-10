// Copyright Epic Games, Inc. All Rights Reserved.

#include "VerdunCraterGenerator.h"
#include "VerdunTerrainParameters.h"
#include "VerdunLocationData.h"
#include "Landscape.h"
#include "LandscapeEdit.h"
#include "Math/RandomStream.h"

AVerdunCraterGenerator::AVerdunCraterGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVerdunCraterGenerator::GenerateCraters()
{
	if (!TerrainParameters)
	{
		UE_LOG(LogTemp, Error, TEXT("TerrainParameters not set. Cannot generate craters."));
		return;
	}

	ClearCraters();

	FRandomStream RandomStream(RandomSeed);

	// Convert battlefield size to centimeters
	FVector2D BattlefieldSizeCm = BattlefieldSizeMeters * 100.0f;

	// Calculate total area in hectares
	float TotalAreaHectares = (BattlefieldSizeMeters.X * BattlefieldSizeMeters.Y) / 10000.0f;

	UE_LOG(LogTemp, Log, TEXT("Generating craters for %.2f hectares..."), TotalAreaHectares);

	// Generate craters in a grid pattern with randomization
	// We'll divide the battlefield into cells and determine density per cell
	const float CellSizeMeters = 100.0f; // 1 hectare cells
	const float CellSizeCm = CellSizeMeters * 100.0f;

	int32 GridX = FMath::CeilToInt(BattlefieldSizeMeters.X / CellSizeMeters);
	int32 GridY = FMath::CeilToInt(BattlefieldSizeMeters.Y / CellSizeMeters);

	for (int32 Y = 0; Y < GridY; Y++)
	{
		for (int32 X = 0; X < GridX; X++)
		{
			// Cell center in world coordinates (relative to battlefield center)
			FVector CellCenter(
				(X * CellSizeMeters - BattlefieldSizeMeters.X / 2.0f + CellSizeMeters / 2.0f) * 100.0f,
				(Y * CellSizeMeters - BattlefieldSizeMeters.Y / 2.0f + CellSizeMeters / 2.0f) * 100.0f,
				0.0f
			);

			// Get crater density for this cell
			float CratersPerHectare = GetCraterDensityAtLocation(CellCenter);

			// Calculate number of craters for this cell (1 hectare)
			int32 NumCraters = FMath::RoundToInt(CratersPerHectare * (1.0f + RandomStream.FRandRange(-0.3f, 0.3f)));

			// Generate craters in this cell
			for (int32 i = 0; i < NumCraters; i++)
			{
				// Random position within cell
				FVector CraterLocation(
					CellCenter.X + RandomStream.FRandRange(-CellSizeCm / 2.0f, CellSizeCm / 2.0f),
					CellCenter.Y + RandomStream.FRandRange(-CellSizeCm / 2.0f, CellSizeCm / 2.0f),
					0.0f // Z will be set by landscape
				);

				FGeneratedCrater Crater = GenerateCraterAtLocation(CraterLocation, RandomStream);
				GeneratedCraters.Add(Crater);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Generated %d craters across the battlefield"), GeneratedCraters.Num());
}

void AVerdunCraterGenerator::ClearCraters()
{
	GeneratedCraters.Empty();
	UE_LOG(LogTemp, Log, TEXT("Cleared all generated craters"));
}

float AVerdunCraterGenerator::GetCraterDensityAtLocation(FVector Location) const
{
	if (!TerrainParameters)
	{
		return 0.0f;
	}

	// Determine intensity based on proximity to high-intensity locations
	if (IsHighIntensityZone(Location))
	{
		return TerrainParameters->CraterDensity.HighIntensity;
	}
	else if (IsMediumIntensityZone(Location))
	{
		return TerrainParameters->CraterDensity.MediumIntensity;
	}
	else
	{
		return TerrainParameters->CraterDensity.LowIntensity;
	}
}

float AVerdunCraterGenerator::CalculateIntensityAtLocation(FVector Location) const
{
	// This could be made more sophisticated with distance-based falloff
	return GetCraterDensityAtLocation(Location);
}

FGeneratedCrater AVerdunCraterGenerator::GenerateCraterAtLocation(FVector Location, FRandomStream& RandomStream) const
{
	FGeneratedCrater Crater;
	Crater.Location = Location;

	if (!TerrainParameters || TerrainParameters->CraterTypes.Num() == 0)
	{
		// Default values
		Crater.DiameterCm = 200.0f;
		Crater.DepthCm = 50.0f;
		Crater.ShellType = TEXT("Unknown");
		Crater.Rotation = RandomStream.FRandRange(0.0f, 360.0f);
		Crater.bWaterFilled = bWaterFilledCraters && RandomStream.FRandRange(0.0f, 100.0f) < WaterFilledPercentage;
		return Crater;
	}

	// Select crater type based on percentage distribution
	float RandValue = RandomStream.FRandRange(0.0f, 100.0f);
	float CumulativePercentage = 0.0f;

	for (const FVerdunCraterParameters& CraterType : TerrainParameters->CraterTypes)
	{
		CumulativePercentage += CraterType.Percentage;
		if (RandValue <= CumulativePercentage)
		{
			Crater.DiameterCm = CraterType.DiameterMeters * 100.0f;
			Crater.DepthCm = CraterType.DepthMeters * 100.0f;
			Crater.ShellType = CraterType.ShellType;
			break;
		}
	}

	// Random rotation for variety
	Crater.Rotation = RandomStream.FRandRange(0.0f, 360.0f);

	// Determine if water-filled
	Crater.bWaterFilled = bWaterFilledCraters && RandomStream.FRandRange(0.0f, 100.0f) < WaterFilledPercentage;

	return Crater;
}

bool AVerdunCraterGenerator::IsHighIntensityZone(FVector Location) const
{
	if (!LocationDatabase || !TerrainParameters)
	{
		return false;
	}

	// High intensity zones are within 1km of major forts
	const float HighIntensityRadiusCm = 100000.0f; // 1km in cm

	TArray<FVerdunLocation> Forts = LocationDatabase->GetFortifications();
	for (const FVerdunLocation& Fort : Forts)
	{
		// Check if this is a major fort (Douaumont, Vaux, Souville)
		if (Fort.Name.Contains(TEXT("Douaumont")) ||
			Fort.Name.Contains(TEXT("Vaux")) ||
			Fort.Name.Contains(TEXT("Souville")) ||
			Fort.Name.Contains(TEXT("Fleury")))
		{
			float Distance = FVector::Dist2D(Location, Fort.UE5Location);
			if (Distance < HighIntensityRadiusCm)
			{
				return true;
			}
		}
	}

	return false;
}

bool AVerdunCraterGenerator::IsMediumIntensityZone(FVector Location) const
{
	if (!LocationDatabase || !TerrainParameters)
	{
		// Default: most of the battlefield is medium intensity
		return true;
	}

	// Medium intensity is the "Right Bank" combat zone
	// Check if location is within the general combat area
	FVector2D Location2D(Location.X, Location.Y);

	// Right Bank center is at approximately X=0, Y=0 (battlefield center)
	// Right Bank extends roughly 3km radius
	const float RightBankRadiusCm = 300000.0f; // 3km in cm

	float DistanceFromCenter = Location2D.Size();

	// If within 3km of center, it's at least medium intensity
	if (DistanceFromCenter < RightBankRadiusCm)
	{
		return true;
	}

	// Check if near other major locations
	const float MediumIntensityRadiusCm = 200000.0f; // 2km in cm

	for (const FVerdunLocation& Loc : LocationDatabase->Locations)
	{
		if (Loc.Type == TEXT("fortification") ||
			Loc.Type == TEXT("destroyed_village") ||
			Loc.Type == TEXT("hill"))
		{
			float Distance = FVector::Dist2D(Location, Loc.UE5Location);
			if (Distance < MediumIntensityRadiusCm)
			{
				return true;
			}
		}
	}

	return false;
}

void AVerdunCraterGenerator::SpawnCraterMeshes(TSubclassOf<AActor> CraterActorClass)
{
	if (!CraterActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("CraterActorClass not set. Cannot spawn crater meshes."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("World not found. Cannot spawn crater meshes."));
		return;
	}

	int32 SpawnedCount = 0;
	for (const FGeneratedCrater& Crater : GeneratedCraters)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(Crater.Location);
		SpawnTransform.SetRotation(FQuat(FRotator(0.0f, Crater.Rotation, 0.0f)));
		SpawnTransform.SetScale3D(FVector(Crater.DiameterCm / 100.0f)); // Scale based on diameter

		AActor* CraterActor = World->SpawnActor<AActor>(CraterActorClass, SpawnTransform, SpawnParams);
		if (CraterActor)
		{
			SpawnedCount++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Spawned %d crater mesh actors"), SpawnedCount);
}

void AVerdunCraterGenerator::ApplyCratersToLandscape(ALandscape* Landscape)
{
	if (!Landscape)
	{
		UE_LOG(LogTemp, Error, TEXT("Landscape not set. Cannot apply craters."));
		return;
	}

	// This would require direct landscape manipulation
	// For now, log a warning that this is a complex operation
	UE_LOG(LogTemp, Warning, TEXT("ApplyCratersToLandscape: Direct landscape modification requires additional implementation."));
	UE_LOG(LogTemp, Warning, TEXT("Consider using landscape sculpting brushes or render target-based height modification."));
	UE_LOG(LogTemp, Log, TEXT("Generated %d craters ready for application"), GeneratedCraters.Num());

	// TODO: Implement landscape height modification using FLandscapeEditDataInterface
	// This would involve:
	// 1. Getting landscape edit interface
	// 2. For each crater, modify height values in a circular pattern
	// 3. Apply smoothing/erosion around crater edges
}
