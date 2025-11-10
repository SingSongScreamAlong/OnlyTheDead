// Copyright Epic Games, Inc. All Rights Reserved.

#include "VerdunTerrainParameters.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"

FVector UVerdunTerrainParameters::ConvertLatLonToUE5Coords(double Latitude, double Longitude, float ElevationMeters) const
{
	// Calculate offset from battlefield center
	double DeltaLat = Latitude - GeographicCenter.Latitude;
	double DeltaLon = Longitude - GeographicCenter.Longitude;

	// Convert to meters
	double XMeters = DeltaLon * MetersPerDegreeLongitude;
	double YMeters = DeltaLat * MetersPerDegreeLatitude;

	// Convert to centimeters (UE5 units)
	// UE5 coordinate system: X = forward (East), Y = right (North), Z = up
	float X = static_cast<float>(XMeters * CentimetersPerMeter);
	float Y = static_cast<float>(YMeters * CentimetersPerMeter);
	float Z = ElevationMeters * CentimetersPerMeter;

	return FVector(X, Y, Z);
}

bool UVerdunTerrainParameters::LoadFromJSON(const FString& FilePath)
{
	// Read JSON file
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load terrain parameters file: %s"), *FilePath);
		return false;
	}

	// Parse JSON
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse terrain parameters JSON: %s"), *FilePath);
		return false;
	}

	// Parse basic info
	TerrainName = JsonObject->GetStringField(TEXT("terrain_name"));
	Description = JsonObject->GetStringField(TEXT("description"));

	// Parse geographic coverage
	if (JsonObject->HasTypedField<EJson::Object>(TEXT("geographic_coverage")))
	{
		TSharedPtr<FJsonObject> GeoCoverage = JsonObject->GetObjectField(TEXT("geographic_coverage"));

		// Parse center
		if (GeoCoverage->HasTypedField<EJson::Object>(TEXT("center")))
		{
			TSharedPtr<FJsonObject> Center = GeoCoverage->GetObjectField(TEXT("center"));
			GeographicCenter.Latitude = Center->GetNumberField(TEXT("latitude"));
			GeographicCenter.Longitude = Center->GetNumberField(TEXT("longitude"));
			GeographicCenter.ElevationMeters = static_cast<float>(Center->GetNumberField(TEXT("elevation_m")));
		}

		// Parse bounds
		if (GeoCoverage->HasTypedField<EJson::Object>(TEXT("bounds")))
		{
			TSharedPtr<FJsonObject> Bounds = GeoCoverage->GetObjectField(TEXT("bounds"));
			GeographicBounds.North = Bounds->GetNumberField(TEXT("north"));
			GeographicBounds.South = Bounds->GetNumberField(TEXT("south"));
			GeographicBounds.East = Bounds->GetNumberField(TEXT("east"));
			GeographicBounds.West = Bounds->GetNumberField(TEXT("west"));
		}

		AreaKm2 = static_cast<float>(GeoCoverage->GetNumberField(TEXT("area_km2")));
	}

	// Parse elevation data
	if (JsonObject->HasTypedField<EJson::Object>(TEXT("elevation_data")))
	{
		TSharedPtr<FJsonObject> ElevData = JsonObject->GetObjectField(TEXT("elevation_data"));

		if (ElevData->HasTypedField<EJson::Object>(TEXT("range_meters")))
		{
			TSharedPtr<FJsonObject> Range = ElevData->GetObjectField(TEXT("range_meters"));
			ElevationMinMeters = static_cast<float>(Range->GetNumberField(TEXT("min")));
			ElevationMaxMeters = static_cast<float>(Range->GetNumberField(TEXT("max")));
			ElevationMeanMeters = static_cast<float>(Range->GetNumberField(TEXT("mean")));
		}
	}

	// Parse surface materials
	if (JsonObject->HasTypedField<EJson::Object>(TEXT("surface_materials")))
	{
		TSharedPtr<FJsonObject> Materials = JsonObject->GetObjectField(TEXT("surface_materials"));

		if (Materials->HasTypedField<EJson::Object>(TEXT("composition_percentage")))
		{
			TSharedPtr<FJsonObject> Composition = Materials->GetObjectField(TEXT("composition_percentage"));
			SurfaceMaterials.Mud = static_cast<float>(Composition->GetNumberField(TEXT("mud")));
			SurfaceMaterials.Chalk = static_cast<float>(Composition->GetNumberField(TEXT("chalk")));
			SurfaceMaterials.DeadGrass = static_cast<float>(Composition->GetNumberField(TEXT("dead_grass")));
			SurfaceMaterials.Rock = static_cast<float>(Composition->GetNumberField(TEXT("rock")));
		}
	}

	// Parse vegetation
	if (JsonObject->HasTypedField<EJson::Object>(TEXT("vegetation")))
	{
		TSharedPtr<FJsonObject> Vegetation = JsonObject->GetObjectField(TEXT("vegetation"));

		if (Vegetation->HasTypedField<EJson::Object>(TEXT("trees")))
		{
			TSharedPtr<FJsonObject> Trees = Vegetation->GetObjectField(TEXT("trees"));
			FString SurvivalRateStr = Trees->GetStringField(TEXT("survival_rate"));
			// Extract numeric value from "1%" string
			SurvivalRateStr.RemoveFromEnd(TEXT("%"));
			TreeSurvivalRate = FCString::Atof(*SurvivalRateStr);
		}

		if (Vegetation->HasTypedField<EJson::Object>(TEXT("ground_vegetation")))
		{
			TSharedPtr<FJsonObject> GroundVeg = Vegetation->GetObjectField(TEXT("ground_vegetation"));
			FString SurvivalRateStr = GroundVeg->GetStringField(TEXT("survival_rate"));
			// Extract numeric value from "5%" string
			SurvivalRateStr.RemoveFromEnd(TEXT("%"));
			GroundVegetationSurvivalRate = FCString::Atof(*SurvivalRateStr);
		}
	}

	// Parse battlefield modifications
	if (JsonObject->HasTypedField<EJson::Object>(TEXT("battlefield_modifications")))
	{
		TSharedPtr<FJsonObject> Modifications = JsonObject->GetObjectField(TEXT("battlefield_modifications"));

		// Parse trenches
		if (Modifications->HasTypedField<EJson::Object>(TEXT("trenches")))
		{
			TSharedPtr<FJsonObject> Trenches = Modifications->GetObjectField(TEXT("trenches"));
			TrenchParameters.TotalLengthKm = static_cast<float>(Trenches->GetNumberField(TEXT("total_length_km")));

			if (Trenches->HasTypedField<EJson::Object>(TEXT("dimensions")))
			{
				TSharedPtr<FJsonObject> Dimensions = Trenches->GetObjectField(TEXT("dimensions"));
				TrenchParameters.WidthMeters = static_cast<float>(Dimensions->GetNumberField(TEXT("width_m")));
				TrenchParameters.DepthMeters = static_cast<float>(Dimensions->GetNumberField(TEXT("depth_m")));
			}

			TrenchParameters.Pattern = Trenches->GetStringField(TEXT("pattern"));
		}

		// Parse shell craters
		if (Modifications->HasTypedField<EJson::Object>(TEXT("shell_craters")))
		{
			TSharedPtr<FJsonObject> Craters = Modifications->GetObjectField(TEXT("shell_craters"));
			TotalCraters = static_cast<int32>(Craters->GetNumberField(TEXT("total_estimated")));

			// Parse density
			if (Craters->HasTypedField<EJson::Object>(TEXT("density_per_hectare")))
			{
				TSharedPtr<FJsonObject> Density = Craters->GetObjectField(TEXT("density_per_hectare"));

				// Parse high intensity (format: "50-100")
				FString HighIntensityStr = Density->GetStringField(TEXT("high_intensity_zones"));
				int32 DashIndex;
				if (HighIntensityStr.FindChar('-', DashIndex))
				{
					FString MinStr = HighIntensityStr.Left(DashIndex);
					FString MaxStr = HighIntensityStr.Mid(DashIndex + 1);
					MaxStr = MaxStr.Left(MaxStr.Find(TEXT(" "))); // Remove any trailing text
					CraterDensity.HighIntensity = (FCString::Atof(*MinStr) + FCString::Atof(*MaxStr)) / 2.0f;
				}

				// Parse medium intensity
				FString MediumIntensityStr = Density->GetStringField(TEXT("medium_intensity_zones"));
				if (MediumIntensityStr.FindChar('-', DashIndex))
				{
					FString MinStr = MediumIntensityStr.Left(DashIndex);
					FString MaxStr = MediumIntensityStr.Mid(DashIndex + 1);
					MaxStr = MaxStr.Left(MaxStr.Find(TEXT(" ")));
					CraterDensity.MediumIntensity = (FCString::Atof(*MinStr) + FCString::Atof(*MaxStr)) / 2.0f;
				}

				// Parse low intensity
				FString LowIntensityStr = Density->GetStringField(TEXT("low_intensity_zones"));
				if (LowIntensityStr.FindChar('-', DashIndex))
				{
					FString MinStr = LowIntensityStr.Left(DashIndex);
					FString MaxStr = LowIntensityStr.Mid(DashIndex + 1);
					MaxStr = MaxStr.Left(MaxStr.Find(TEXT(" ")));
					CraterDensity.LowIntensity = (FCString::Atof(*MinStr) + FCString::Atof(*MaxStr)) / 2.0f;
				}
			}

			// Parse crater sizes
			if (Craters->HasTypedField<EJson::Array>(TEXT("sizes_by_shell_type")))
			{
				TArray<TSharedPtr<FJsonValue>> SizesArray = Craters->GetArrayField(TEXT("sizes_by_shell_type"));
				for (const TSharedPtr<FJsonValue>& SizeValue : SizesArray)
				{
					TSharedPtr<FJsonObject> SizeObj = SizeValue->AsObject();

					FVerdunCraterParameters CraterParams;
					CraterParams.ShellType = SizeObj->GetStringField(TEXT("shell"));
					CraterParams.DiameterMeters = static_cast<float>(SizeObj->GetNumberField(TEXT("diameter_m")));
					CraterParams.DepthMeters = static_cast<float>(SizeObj->GetNumberField(TEXT("depth_m")));
					CraterParams.Percentage = static_cast<float>(SizeObj->GetNumberField(TEXT("percentage")));
					CraterParams.Note = SizeObj->GetStringField(TEXT("note"));

					CraterTypes.Add(CraterParams);
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Successfully loaded Verdun terrain parameters from: %s"), *FilePath);
	return true;
}
