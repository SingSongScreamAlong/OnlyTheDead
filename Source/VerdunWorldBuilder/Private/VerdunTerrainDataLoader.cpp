// Verdun World Builder Plugin
// Terrain data loader implementation

#include "VerdunTerrainDataLoader.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "JsonUtilities.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Math/UnrealMathUtility.h"

UVerdunTerrainDataLoader::UVerdunTerrainDataLoader()
{
}

bool UVerdunTerrainDataLoader::LoadTerrainParameters(const FString& FilePath, FVerdunTerrainData& OutTerrainData)
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load terrain parameters from: %s"), *FilePath);
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse terrain parameters JSON"));
		return false;
	}

	// Parse terrain name
	OutTerrainData.TerrainName = JsonObject->GetStringField(TEXT("terrain_name"));

	// Parse geographic coverage
	if (JsonObject->HasField(TEXT("geographic_coverage")))
	{
		TSharedPtr<FJsonObject> Coverage = JsonObject->GetObjectField(TEXT("geographic_coverage"));

		// Parse center
		if (Coverage->HasField(TEXT("center")))
		{
			TSharedPtr<FJsonObject> Center = Coverage->GetObjectField(TEXT("center"));
			OutTerrainData.BattlefieldCenter.Latitude = Center->GetNumberField(TEXT("latitude"));
			OutTerrainData.BattlefieldCenter.Longitude = Center->GetNumberField(TEXT("longitude"));
			OutTerrainData.BattlefieldCenter.ElevationMeters = Center->GetNumberField(TEXT("elevation_m"));
		}

		// Parse bounds
		if (Coverage->HasField(TEXT("bounds")))
		{
			TSharedPtr<FJsonObject> Bounds = Coverage->GetObjectField(TEXT("bounds"));
			OutTerrainData.GeographicBounds.North = Bounds->GetNumberField(TEXT("north"));
			OutTerrainData.GeographicBounds.South = Bounds->GetNumberField(TEXT("south"));
			OutTerrainData.GeographicBounds.East = Bounds->GetNumberField(TEXT("east"));
			OutTerrainData.GeographicBounds.West = Bounds->GetNumberField(TEXT("west"));
		}
	}

	// Parse elevation data (Tier 1 only - documented ranges)
	if (JsonObject->HasField(TEXT("elevation_data")))
	{
		TSharedPtr<FJsonObject> ElevData = JsonObject->GetObjectField(TEXT("elevation_data"));
		if (ElevData->HasField(TEXT("range_meters")))
		{
			TSharedPtr<FJsonObject> Range = ElevData->GetObjectField(TEXT("range_meters"));
			OutTerrainData.MinElevationMeters = Range->GetNumberField(TEXT("min"));
			OutTerrainData.MaxElevationMeters = Range->GetNumberField(TEXT("max"));
			OutTerrainData.MeanElevationMeters = Range->GetNumberField(TEXT("mean"));
		}
	}

	// Parse surface materials (Tier 1 - documented from photos/testimonies)
	if (JsonObject->HasField(TEXT("surface_materials")))
	{
		TSharedPtr<FJsonObject> Materials = JsonObject->GetObjectField(TEXT("surface_materials"));
		if (Materials->HasField(TEXT("composition_percentage")))
		{
			TSharedPtr<FJsonObject> Composition = Materials->GetObjectField(TEXT("composition_percentage"));
			OutTerrainData.SurfaceMaterials.MudPercentage = Composition->GetNumberField(TEXT("mud"));
			OutTerrainData.SurfaceMaterials.ChalkPercentage = Composition->GetNumberField(TEXT("chalk"));
			OutTerrainData.SurfaceMaterials.DeadGrassPercentage = Composition->GetNumberField(TEXT("dead_grass"));
			OutTerrainData.SurfaceMaterials.RockPercentage = Composition->GetNumberField(TEXT("rock"));
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Successfully loaded terrain parameters: %s"), *OutTerrainData.TerrainName);
	UE_LOG(LogTemp, Log, TEXT("  Center: %.4f°N, %.4f°E, %.0fm"),
		OutTerrainData.BattlefieldCenter.Latitude,
		OutTerrainData.BattlefieldCenter.Longitude,
		OutTerrainData.BattlefieldCenter.ElevationMeters);
	UE_LOG(LogTemp, Log, TEXT("  Elevation: %.0f-%.0fm (mean: %.0fm)"),
		OutTerrainData.MinElevationMeters,
		OutTerrainData.MaxElevationMeters,
		OutTerrainData.MeanElevationMeters);

	return true;
}

bool UVerdunTerrainDataLoader::LoadLocationsDatabase(const FString& FilePath, const FVerdunBattlefieldCenter& BattlefieldCenter, TArray<FVerdunLocation>& OutLocations)
{
	FString CsvString;
	if (!FFileHelper::LoadFileToString(CsvString, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load locations database from: %s"), *FilePath);
		return false;
	}

	TArray<FString> Lines;
	CsvString.ParseIntoArrayLines(Lines);

	if (Lines.Num() < 2)
	{
		UE_LOG(LogTemp, Error, TEXT("Locations database is empty or invalid"));
		return false;
	}

	// Skip header line
	for (int32 i = 1; i < Lines.Num(); i++)
	{
		TArray<FString> Fields = ParseCSVLine(Lines[i]);

		if (Fields.Num() < 9)
		{
			UE_LOG(LogTemp, Warning, TEXT("Skipping invalid line %d: not enough fields"), i);
			continue;
		}

		FVerdunLocation Location;
		Location.Name = Fields[0].TrimStartAndEnd();
		Location.Type = ParseLocationType(Fields[1].TrimStartAndEnd());
		Location.Latitude = FCString::Atod(*Fields[2]);
		Location.Longitude = FCString::Atod(*Fields[3]);
		Location.ElevationMeters = FCString::Atof(*Fields[4]);
		Location.Significance = Fields[5].TrimStartAndEnd();
		Location.Phase = Fields[6].TrimStartAndEnd();
		Location.DateKey = Fields[7].TrimStartAndEnd();
		Location.Notes = Fields[8].TrimStartAndEnd();

		// Convert to world coordinates
		Location.WorldPosition = ConvertGeoToWorld(
			Location.Latitude,
			Location.Longitude,
			Location.ElevationMeters,
			BattlefieldCenter
		);

		OutLocations.Add(Location);
	}

	UE_LOG(LogTemp, Log, TEXT("Successfully loaded %d locations from database"), OutLocations.Num());

	return true;
}

FVector UVerdunTerrainDataLoader::ConvertGeoToWorld(double Latitude, double Longitude, float ElevationMeters, const FVerdunBattlefieldCenter& BattlefieldCenter, float Scale)
{
	// Simple equirectangular projection (suitable for small areas like Verdun)
	// More accurate projections (UTM) would be needed for larger areas

	const double MetersPerDegreeLat = 111320.0; // Approximately constant
	const double CenterLatRad = FMath::DegreesToRadians(BattlefieldCenter.Latitude);
	const double MetersPerDegreeLon = 111320.0 * FMath::Cos(CenterLatRad);

	// Calculate offset from center in meters
	double LatDiff = (Latitude - BattlefieldCenter.Latitude) * MetersPerDegreeLat;
	double LonDiff = (Longitude - BattlefieldCenter.Longitude) * MetersPerDegreeLon;

	// Convert to Unreal coordinates (cm)
	// X = East/West (longitude), Y = North/South (latitude), Z = Up (elevation)
	float X = LonDiff * Scale;
	float Y = LatDiff * Scale;
	float Z = ElevationMeters * Scale;

	return FVector(X, Y, Z);
}

bool UVerdunTerrainDataLoader::LoadCompleteTerrainData(const FString& TerrainParamsPath, const FString& LocationsDBPath, FVerdunTerrainData& OutTerrainData)
{
	// Load terrain parameters first
	if (!LoadTerrainParameters(TerrainParamsPath, OutTerrainData))
	{
		return false;
	}

	// Then load locations with converted coordinates
	TArray<FVerdunLocation> Locations;
	if (!LoadLocationsDatabase(LocationsDBPath, OutTerrainData.BattlefieldCenter, Locations))
	{
		return false;
	}

	OutTerrainData.Locations = Locations;

	UE_LOG(LogTemp, Log, TEXT("Complete terrain data loaded successfully"));
	UE_LOG(LogTemp, Log, TEXT("  Terrain: %s"), *OutTerrainData.TerrainName);
	UE_LOG(LogTemp, Log, TEXT("  Locations: %d"), OutTerrainData.Locations.Num());
	UE_LOG(LogTemp, Log, TEXT("  Surface: %.0f%% mud, %.0f%% chalk, %.0f%% grass, %.0f%% rock"),
		OutTerrainData.SurfaceMaterials.MudPercentage,
		OutTerrainData.SurfaceMaterials.ChalkPercentage,
		OutTerrainData.SurfaceMaterials.DeadGrassPercentage,
		OutTerrainData.SurfaceMaterials.RockPercentage);

	return true;
}

EVerdunLocationType UVerdunTerrainDataLoader::ParseLocationType(const FString& TypeString)
{
	FString Type = TypeString.ToLower();

	if (Type == TEXT("fortification")) return EVerdunLocationType::Fortification;
	if (Type == TEXT("hill")) return EVerdunLocationType::Hill;
	if (Type == TEXT("destroyed_village")) return EVerdunLocationType::DestroyedVillage;
	if (Type == TEXT("village")) return EVerdunLocationType::Village;
	if (Type == TEXT("woods")) return EVerdunLocationType::Woods;
	if (Type == TEXT("ravine")) return EVerdunLocationType::Ravine;
	if (Type == TEXT("river")) return EVerdunLocationType::River;
	if (Type == TEXT("ridge")) return EVerdunLocationType::Ridge;
	if (Type == TEXT("road")) return EVerdunLocationType::Road;
	if (Type == TEXT("trench")) return EVerdunLocationType::Trench;
	if (Type == TEXT("artillery")) return EVerdunLocationType::Artillery;
	if (Type == TEXT("observation")) return EVerdunLocationType::Observation;
	if (Type == TEXT("command")) return EVerdunLocationType::Command;

	return EVerdunLocationType::Other;
}

TArray<FString> UVerdunTerrainDataLoader::ParseCSVLine(const FString& Line)
{
	TArray<FString> Fields;
	FString CurrentField;
	bool bInQuotes = false;

	for (int32 i = 0; i < Line.Len(); i++)
	{
		TCHAR Char = Line[i];

		if (Char == TEXT('"'))
		{
			bInQuotes = !bInQuotes;
		}
		else if (Char == TEXT(',') && !bInQuotes)
		{
			Fields.Add(CurrentField);
			CurrentField.Empty();
		}
		else
		{
			CurrentField.AppendChar(Char);
		}
	}

	// Add last field
	Fields.Add(CurrentField);

	return Fields;
}
