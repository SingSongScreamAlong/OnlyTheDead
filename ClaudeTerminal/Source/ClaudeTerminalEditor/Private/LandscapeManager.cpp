// Copyright Epic Games, Inc. All Rights Reserved.

#include "LandscapeManager.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeEditorObject.h"
#include "LandscapeEdit.h"
#include "LandscapeDataAccess.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "LandscapeStreamingProxy.h"

FLandscapeManager::FLandscapeManager()
{
}

FLandscapeManager::~FLandscapeManager()
{
}

ALandscape* FLandscapeManager::CreateLandscapeFromHeightmap(
	UWorld* World,
	const FString& HeightmapPath,
	const FVector& Location,
	const FVector& Scale,
	UMaterialInterface* Material,
	FString& OutLog)
{
	if (!World)
	{
		OutLog = TEXT("Error: Invalid world.");
		return nullptr;
	}

	// Load heightmap
	TArray<uint16> HeightData;
	int32 Width, Height;
	if (!HeightmapLoader.LoadHeightmap(HeightmapPath, HeightData, Width, Height))
	{
		OutLog = FString::Printf(TEXT("Error: Failed to load heightmap from '%s'."), *HeightmapPath);
		return nullptr;
	}

	// Validate
	if (!HeightmapLoader.ValidateHeightmap(HeightData, Width, Height))
	{
		OutLog = TEXT("Error: Invalid heightmap data.");
		return nullptr;
	}

	// Get statistics
	uint16 MinHeight, MaxHeight;
	float AvgHeight;
	HeightmapLoader.GetHeightmapStats(HeightData, MinHeight, MaxHeight, AvgHeight);

	// Calculate landscape dimensions
	int32 QuadsPerComponent, ComponentsX, ComponentsY;
	CalculateLandscapeSize(Width, Height, QuadsPerComponent, ComponentsX, ComponentsY);

	// Create landscape
	TMap<FGuid, TArray<uint16>> HeightmapDataPerLayers;
	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;

	// Spawn landscape actor
	ALandscape* Landscape = World->SpawnActor<ALandscape>(Location, FRotator::ZeroRotator);
	if (!Landscape)
	{
		OutLog = TEXT("Error: Failed to spawn landscape actor.");
		return nullptr;
	}

	// Configure landscape
	Landscape->SetActorScale3D(Scale);

	if (Material)
	{
		Landscape->LandscapeMaterial = Material;
	}

	// Set up landscape components
	Landscape->ComponentSizeQuads = QuadsPerComponent;
	Landscape->SubsectionSizeQuads = QuadsPerComponent / 2;
	Landscape->NumSubsections = 4;

	// Import heightmap data
	TArray<uint16> LandscapeData;
	if (!ConvertHeightmapData(HeightData, Width, Height, LandscapeData))
	{
		OutLog = TEXT("Error: Failed to convert heightmap data.");
		World->DestroyActor(Landscape);
		return nullptr;
	}

	// Create landscape info and import
	Landscape->Import(
		FGuid::NewGuid(),
		0, 0,
		Width - 1, Height - 1,
		QuadsPerComponent,
		QuadsPerComponent / 2,
		4,
		LandscapeData.GetData(),
		nullptr,
		TArray<FLandscapeImportLayerInfo>(),
		ELandscapeImportAlphamapType::Additive
	);

	OutLog = FString::Printf(
		TEXT("Successfully created landscape from '%s'\n"
		     "Size: %dx%d pixels (%d components)\n"
		     "Height range: %d - %d (%.1f average)\n"
		     "Location: %s, Scale: %s"),
		*FPaths::GetCleanFilename(HeightmapPath),
		Width, Height, ComponentsX * ComponentsY,
		MinHeight, MaxHeight, AvgHeight,
		*Location.ToString(), *Scale.ToString()
	);

	UE_LOG(LogTemp, Log, TEXT("%s"), *OutLog);
	return Landscape;
}

ALandscape* FLandscapeManager::CreateLandscapeFromGPSData(
	UWorld* World,
	const FString& HeightmapPath,
	double GPSOriginLat,
	double GPSOriginLon,
	double MetersPerPixel,
	UMaterialInterface* Material,
	FString& OutLog)
{
	// Set GPS reference point
	GPSConverter.SetReferencePoint(GPSOriginLat, GPSOriginLon);

	// Load heightmap to get dimensions
	TArray<uint16> HeightData;
	int32 Width, Height;
	if (!HeightmapLoader.LoadHeightmap(HeightmapPath, HeightData, Width, Height))
	{
		OutLog = FString::Printf(TEXT("Error: Failed to load heightmap from '%s'."), *HeightmapPath);
		return nullptr;
	}

	// Calculate real-world extent
	double ExtentMetersX = Width * MetersPerPixel;
	double ExtentMetersY = Height * MetersPerPixel;

	// Convert GPS origin to UE5 coordinates
	double AvgAltitude = 250.0; // Default altitude if not specified
	FVector UE5Location = GPSConverter.GPSToUE5(GPSOriginLat, GPSOriginLon, AvgAltitude);

	// Calculate scale: convert meters to UE5 units (cm)
	FVector Scale;
	Scale.X = MetersPerPixel * 100.0; // meters → cm
	Scale.Y = MetersPerPixel * 100.0;
	Scale.Z = 100.0; // Default Z scale (can adjust based on heightmap range)

	// Create landscape
	ALandscape* Landscape = CreateLandscapeFromHeightmap(World, HeightmapPath, UE5Location, Scale, Material, OutLog);

	if (Landscape)
	{
		OutLog += FString::Printf(
			TEXT("\n\nGPS Reference: %.6f°N, %.6f°E\n"
			     "Real-world extent: %.1f × %.1f meters\n"
			     "UE5 location: %s"),
			GPSOriginLat, GPSOriginLon,
			ExtentMetersX, ExtentMetersY,
			*UE5Location.ToString()
		);
	}

	return Landscape;
}

bool FLandscapeManager::ModifyLandscapeTerrain(
	UWorld* World,
	const FVector& Location,
	float Radius,
	float Strength,
	const FString& Operation,
	FString& OutLog)
{
	ALandscape* Landscape = FindLandscapeAtLocation(World, Location);
	if (!Landscape)
	{
		OutLog = TEXT("Error: No landscape found at location.");
		return false;
	}

	ULandscapeInfo* LandscapeInfo = GetLandscapeInfo(Landscape);
	if (!LandscapeInfo)
	{
		OutLog = TEXT("Error: Could not get landscape info.");
		return false;
	}

	// Create modification function based on operation
	TFunction<float(float, float)> ModFunc;

	if (Operation == TEXT("raise"))
	{
		ModFunc = [Strength](float CurrentHeight, float Distance) -> float
		{
			float Falloff = 1.0f - FMath::Clamp(Distance, 0.0f, 1.0f);
			return CurrentHeight + (Strength * 100.0f * Falloff);
		};
	}
	else if (Operation == TEXT("lower"))
	{
		ModFunc = [Strength](float CurrentHeight, float Distance) -> float
		{
			float Falloff = 1.0f - FMath::Clamp(Distance, 0.0f, 1.0f);
			return CurrentHeight - (Strength * 100.0f * Falloff);
		};
	}
	else if (Operation == TEXT("smooth"))
	{
		// Smoothing requires neighbor sampling - simplified version
		ModFunc = [](float CurrentHeight, float Distance) -> float
		{
			return CurrentHeight; // Placeholder - need proper smoothing kernel
		};
	}
	else if (Operation == TEXT("flatten"))
	{
		float TargetHeight = Location.Z;
		ModFunc = [TargetHeight, Strength](float CurrentHeight, float Distance) -> float
		{
			float Falloff = 1.0f - FMath::Clamp(Distance, 0.0f, 1.0f);
			return FMath::Lerp(CurrentHeight, TargetHeight, Strength * Falloff);
		};
	}
	else
	{
		OutLog = FString::Printf(TEXT("Error: Unknown operation '%s'. Use: raise, lower, smooth, or flatten."), *Operation);
		return false;
	}

	// Apply modification
	if (!ApplyTerrainModification(LandscapeInfo, Location, Radius, ModFunc))
	{
		OutLog = TEXT("Error: Failed to apply terrain modification.");
		return false;
	}

	OutLog = FString::Printf(
		TEXT("Successfully modified landscape: %s operation at %s (radius: %.1fm, strength: %.2f)"),
		*Operation, *Location.ToString(), Radius / 100.0f, Strength
	);

	return true;
}

bool FLandscapeManager::CreateCrater(
	UWorld* World,
	const FVector& Location,
	float Radius,
	float Depth,
	FString& OutLog)
{
	ALandscape* Landscape = FindLandscapeAtLocation(World, Location);
	if (!Landscape)
	{
		OutLog = TEXT("Error: No landscape found at location.");
		return false;
	}

	ULandscapeInfo* LandscapeInfo = GetLandscapeInfo(Landscape);
	if (!LandscapeInfo)
	{
		OutLog = TEXT("Error: Could not get landscape info.");
		return false;
	}

	// Crater deformation function
	TFunction<float(float, float)> CraterFunc = [this, Radius, Depth](float CurrentHeight, float Distance) -> float
	{
		float Deformation = CalculateCraterDeformation(Distance, 1.0f, Depth);
		return CurrentHeight + Deformation;
	};

	if (!ApplyTerrainModification(LandscapeInfo, Location, Radius, CraterFunc))
	{
		OutLog = TEXT("Error: Failed to create crater.");
		return false;
	}

	OutLog = FString::Printf(
		TEXT("Successfully created crater at %s (radius: %.1fm, depth: %.1fm)"),
		*Location.ToString(), Radius / 100.0f, Depth / 100.0f
	);

	return true;
}

bool FLandscapeManager::CreateTrench(
	UWorld* World,
	const FVector& StartLocation,
	const FVector& EndLocation,
	float Width,
	float Depth,
	FString& OutLog)
{
	ALandscape* Landscape = FindLandscapeAtLocation(World, StartLocation);
	if (!Landscape)
	{
		OutLog = TEXT("Error: No landscape found at start location.");
		return false;
	}

	// Calculate trench parameters
	FVector Direction = (EndLocation - StartLocation).GetSafeNormal();
	float Length = FVector::Dist(StartLocation, EndLocation);
	int32 NumSamples = FMath::CeilToInt(Length / 100.0f); // Sample every meter

	// Create trench by applying crater-like deformations along the line
	for (int32 i = 0; i <= NumSamples; i++)
	{
		float Alpha = static_cast<float>(i) / NumSamples;
		FVector SampleLocation = FMath::Lerp(StartLocation, EndLocation, Alpha);

		// Create rectangular depression
		FString TempLog;
		if (!CreateCrater(World, SampleLocation, Width, Depth, TempLog))
		{
			OutLog = FString::Printf(TEXT("Error: Failed to create trench segment at %s."), *SampleLocation.ToString());
			return false;
		}
	}

	OutLog = FString::Printf(
		TEXT("Successfully created trench from %s to %s (width: %.1fm, depth: %.1fm, length: %.1fm)"),
		*StartLocation.ToString(), *EndLocation.ToString(),
		Width / 100.0f, Depth / 100.0f, Length / 100.0f
	);

	return true;
}

ALandscape* FLandscapeManager::FindLandscapeAtLocation(UWorld* World, const FVector& Location)
{
	if (!World)
	{
		return nullptr;
	}

	// Find landscape actors
	for (TActorIterator<ALandscape> It(World); It; ++It)
	{
		ALandscape* Landscape = *It;
		if (Landscape)
		{
			// Check if location is within landscape bounds
			FBox Bounds = Landscape->GetComponentsBoundingBox();
			if (Bounds.IsInside(Location))
			{
				return Landscape;
			}
		}
	}

	return nullptr;
}

ULandscapeInfo* FLandscapeManager::GetLandscapeInfo(ALandscape* Landscape)
{
	if (!Landscape)
	{
		return nullptr;
	}

	return Landscape->GetLandscapeInfo();
}

bool FLandscapeManager::ConvertHeightmapData(
	const TArray<uint16>& HeightData,
	int32 Width,
	int32 Height,
	TArray<uint16>& OutLandscapeData)
{
	// UE5 landscapes expect specific dimensions - may need to resample
	// For now, direct copy if dimensions are compatible
	OutLandscapeData = HeightData;
	return true;
}

void FLandscapeManager::CalculateLandscapeSize(
	int32 HeightmapWidth,
	int32 HeightmapHeight,
	int32& OutQuadsPerComponent,
	int32& OutComponentsX,
	int32& OutComponentsY)
{
	// UE5 landscape components must be power-of-2 + 1
	// Common sizes: 7, 15, 31, 63, 127, 255
	// Components: typically 1x1, 2x2, 4x4, 8x8, etc.

	// Find best fit quads per component
	int32 ValidSizes[] = {7, 15, 31, 63, 127, 255};
	OutQuadsPerComponent = 63; // Default

	for (int32 Size : ValidSizes)
	{
		if (HeightmapWidth % Size == 0 && HeightmapHeight % Size == 0)
		{
			OutQuadsPerComponent = Size;
			break;
		}
	}

	// Calculate number of components
	OutComponentsX = FMath::Max(1, HeightmapWidth / OutQuadsPerComponent);
	OutComponentsY = FMath::Max(1, HeightmapHeight / OutQuadsPerComponent);

	UE_LOG(LogTemp, Log, TEXT("Landscape size: %d quads/component, %dx%d components"),
		OutQuadsPerComponent, OutComponentsX, OutComponentsY);
}

bool FLandscapeManager::ApplyTerrainModification(
	ULandscapeInfo* LandscapeInfo,
	const FVector& Location,
	float Radius,
	TFunction<float(float, float)> ModificationFunc)
{
	if (!LandscapeInfo)
	{
		return false;
	}

	// Use FLandscapeEditDataInterface to modify heightmap
	FLandscapeEditDataInterface LandscapeEdit(LandscapeInfo);

	// Calculate affected area
	int32 MinX = FMath::FloorToInt((Location.X - Radius) / 100.0f);
	int32 MinY = FMath::FloorToInt((Location.Y - Radius) / 100.0f);
	int32 MaxX = FMath::CeilToInt((Location.X + Radius) / 100.0f);
	int32 MaxY = FMath::CeilToInt((Location.Y + Radius) / 100.0f);

	// Modify heights
	for (int32 Y = MinY; Y <= MaxY; Y++)
	{
		for (int32 X = MinX; X <= MaxX; X++)
		{
			FVector SampleLocation(X * 100.0f, Y * 100.0f, 0.0f);
			float Distance = FVector::Dist2D(Location, SampleLocation) / Radius;

			if (Distance <= 1.0f)
			{
				// Get current height
				uint16 CurrentHeight = 32768; // Default mid-range
				LandscapeEdit.GetHeightData(X, Y, X, Y, (uint16*)&CurrentHeight, 0);

				// Apply modification
				float NewHeight = ModificationFunc(static_cast<float>(CurrentHeight), Distance);
				uint16 FinalHeight = FMath::Clamp(static_cast<uint16>(NewHeight), (uint16)0, (uint16)65535);

				// Set new height
				LandscapeEdit.SetHeightData(X, Y, X, Y, &FinalHeight, 0, false);
			}
		}
	}

	// Flush changes
	LandscapeEdit.Flush();
	return true;
}

float FLandscapeManager::CalculateCraterDeformation(float DistanceFromCenter, float Radius, float Depth)
{
	if (DistanceFromCenter > Radius)
	{
		return 0.0f;
	}

	// Realistic crater profile: bowl-shaped with raised rim
	float NormalizedDist = DistanceFromCenter / Radius;

	// Inner bowl (depression)
	if (NormalizedDist < 0.7f)
	{
		float BowlFactor = 1.0f - (NormalizedDist / 0.7f);
		return -Depth * FMath::Pow(BowlFactor, 2.0f);
	}
	// Outer rim (raised edge)
	else
	{
		float RimFactor = (NormalizedDist - 0.7f) / 0.3f;
		return Depth * 0.15f * (1.0f - RimFactor);
	}
}
