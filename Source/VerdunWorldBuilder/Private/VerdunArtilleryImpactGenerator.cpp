// Verdun World Builder Plugin
// Artillery impact generator implementation

#include "VerdunArtilleryImpactGenerator.h"
#include "Landscape.h"
#include "LandscapeEdit.h"
#include "Math/UnrealMathUtility.h"

UVerdunDefaultArtilleryGenerator::UVerdunDefaultArtilleryGenerator()
	: RandomSeed(12345)
	, SmallCraterPercentage(60.0f)
	, MediumCraterPercentage(25.0f)
	, LargeCraterPercentage(10.0f)
	, HugeCraterPercentage(5.0f)
{
}

void UVerdunDefaultArtilleryGenerator::GenerateShellImpacts_Implementation(FVector ZoneCenter, float ZoneRadius, float TargetDensity, TArray<FVerdunShellImpact>& OutImpacts)
{
	// Calculate area in hectares (1 hectare = 10,000 m²)
	float ZoneRadiusMeters = ZoneRadius / 100.0f; // Convert cm to meters
	float AreaSquareMeters = PI * ZoneRadiusMeters * ZoneRadiusMeters;
	float AreaHectares = AreaSquareMeters / 10000.0f;

	// Calculate total craters needed
	int32 TotalCraters = FMath::RoundToInt(AreaHectares * TargetDensity);

	UE_LOG(LogTemp, Log, TEXT("Generating %d shell impacts in %.1fm radius (%.2f hectares, density: %.0f/hectare)"),
		TotalCraters, ZoneRadiusMeters, AreaHectares, TargetDensity);

	// Initialize random stream with seed
	FRandomStream RandomStream(RandomSeed);

	OutImpacts.Empty();

	for (int32 i = 0; i < TotalCraters; i++)
	{
		FVerdunShellImpact Impact;

		// Random position within circle
		float Angle = RandomStream.FRand() * 2.0f * PI;
		float Distance = FMath::Sqrt(RandomStream.FRand()) * ZoneRadius; // Sqrt for uniform distribution
		FVector Offset(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.0f);
		Impact.ImpactLocation = ZoneCenter + Offset;

		// Determine shell type based on percentages
		float Roll = RandomStream.FRand() * 100.0f;
		if (Roll < SmallCraterPercentage)
		{
			Impact.ShellType = EVerdunShellType::Field75mm;
		}
		else if (Roll < SmallCraterPercentage + MediumCraterPercentage)
		{
			Impact.ShellType = EVerdunShellType::Howitzer155mm;
		}
		else if (Roll < SmallCraterPercentage + MediumCraterPercentage + LargeCraterPercentage)
		{
			Impact.ShellType = EVerdunShellType::Heavy305mm;
		}
		else
		{
			Impact.ShellType = EVerdunShellType::SuperHeavy420mm;
		}

		// Get crater dimensions
		GetCraterDimensionsForShellType_Implementation(Impact.ShellType, Impact.CraterDiameterMeters, Impact.CraterDepthMeters);

		// Add some randomization (±15%)
		Impact.CraterDiameterMeters *= RandomStream.FRandRange(0.85f, 1.15f);
		Impact.CraterDepthMeters *= RandomStream.FRandRange(0.85f, 1.15f);

		Impact.TimeStamp = i; // Simple sequential timestamp

		OutImpacts.Add(Impact);
	}

	UE_LOG(LogTemp, Log, TEXT("Generated %d shell impacts"), OutImpacts.Num());
}

void UVerdunDefaultArtilleryGenerator::ApplyCraterToLandscape_Implementation(const FVerdunShellImpact& Impact, ALandscape* Landscape)
{
	if (!Landscape)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot apply crater: Landscape is null"));
		return;
	}

	// This is a placeholder implementation
	// In a real implementation, you would:
	// 1. Get the landscape heightmap
	// 2. Calculate which heightmap points are within the crater radius
	// 3. Lower those points by the crater depth with a smooth falloff
	// 4. Update the landscape

	UE_LOG(LogTemp, Log, TEXT("Applying crater at %s (%.1fm diameter, %.1fm deep) - [PLACEHOLDER]"),
		*Impact.ImpactLocation.ToString(),
		Impact.CraterDiameterMeters,
		Impact.CraterDepthMeters);

	// TODO: Implement actual landscape modification
	// You can use FLandscapeEditDataInterface for this
	// Example:
	// FLandscapeEditDataInterface LandscapeEdit(Landscape->GetLandscapeInfo());
	// Modify heightmap data...
	// LandscapeEdit.Flush();
}

void UVerdunDefaultArtilleryGenerator::GenerateTrench_Implementation(FVector StartPoint, FVector EndPoint, float TrenchWidth, float TrenchDepth, float ZigzagFrequency, ALandscape* Landscape)
{
	if (!Landscape)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot generate trench: Landscape is null"));
		return;
	}

	// Placeholder implementation
	float DistanceMeters = (EndPoint - StartPoint).Size() / 100.0f;

	UE_LOG(LogTemp, Log, TEXT("Generating trench from %s to %s (%.1fm long, %.0fcm wide, %.0fcm deep) - [PLACEHOLDER]"),
		*StartPoint.ToString(),
		*EndPoint.ToString(),
		DistanceMeters,
		TrenchWidth,
		TrenchDepth);

	// TODO: Implement actual trench generation
	// 1. Generate spline path with zigzag pattern
	// 2. Sample points along spline
	// 3. For each point, lower landscape in trench width area
	// 4. Create vertical walls if needed
}

void UVerdunDefaultArtilleryGenerator::GetCraterDimensionsForShellType_Implementation(EVerdunShellType ShellType, float& OutDiameterMeters, float& OutDepthMeters)
{
	// Historical data from terrain_parameters.json
	switch (ShellType)
	{
		case EVerdunShellType::Field75mm:
			OutDiameterMeters = 2.0f;
			OutDepthMeters = 0.5f;
			break;

		case EVerdunShellType::Howitzer155mm:
			OutDiameterMeters = 6.0f;
			OutDepthMeters = 1.5f;
			break;

		case EVerdunShellType::Heavy305mm:
			OutDiameterMeters = 15.0f;
			OutDepthMeters = 3.0f;
			break;

		case EVerdunShellType::SuperHeavy420mm:
			OutDiameterMeters = 30.0f;
			OutDepthMeters = 6.0f;
			break;

		default:
			OutDiameterMeters = 2.0f;
			OutDepthMeters = 0.5f;
			break;
	}
}
