// Copyright Epic Games, Inc. All Rights Reserved.

#include "VerdunLocationData.h"
#include "VerdunTerrainParameters.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"

bool UVerdunLocationDatabase::LoadFromCSV(const FString& FilePath)
{
	// Read CSV file
	FString CsvString;
	if (!FFileHelper::LoadFileToString(CsvString, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load locations CSV file: %s"), *FilePath);
		return false;
	}

	// Split into lines
	TArray<FString> Lines;
	CsvString.ParseIntoArrayLines(Lines);

	if (Lines.Num() < 2) // Need at least header + 1 data row
	{
		UE_LOG(LogTemp, Error, TEXT("CSV file is empty or has no data: %s"), *FilePath);
		return false;
	}

	// Clear existing locations
	Locations.Empty();

	// Parse each line (skip header)
	for (int32 i = 1; i < Lines.Num(); i++)
	{
		FString Line = Lines[i];
		if (Line.IsEmpty())
		{
			continue;
		}

		// Split by comma (handle quoted strings that contain commas)
		TArray<FString> Fields;
		bool bInQuotes = false;
		FString CurrentField;

		for (int32 CharIndex = 0; CharIndex < Line.Len(); CharIndex++)
		{
			TCHAR Char = Line[CharIndex];

			if (Char == '"')
			{
				bInQuotes = !bInQuotes;
			}
			else if (Char == ',' && !bInQuotes)
			{
				Fields.Add(CurrentField);
				CurrentField.Empty();
			}
			else
			{
				CurrentField.AppendChar(Char);
			}
		}
		// Add the last field
		Fields.Add(CurrentField);

		// CSV format: name,type,latitude,longitude,elevation_m,significance,phase,date_key,notes
		if (Fields.Num() >= 5) // At minimum we need name, type, lat, lon, elevation
		{
			FVerdunLocation Location;
			Location.Name = Fields[0].TrimStartAndEnd();
			Location.Type = Fields[1].TrimStartAndEnd();

			// Handle "woods,woods,49.2331" case where lat is in wrong column
			int32 LatIndex = 2;
			int32 LonIndex = 3;
			int32 ElevIndex = 4;

			// Check if field 2 is a duplicate type (like "woods,woods")
			if (Fields[2].TrimStartAndEnd() == Location.Type)
			{
				LatIndex = 3;
				LonIndex = 4;
				ElevIndex = 5;
			}

			if (Fields.Num() > LatIndex)
			{
				Location.Latitude = FCString::Atod(*Fields[LatIndex].TrimStartAndEnd());
			}
			if (Fields.Num() > LonIndex)
			{
				Location.Longitude = FCString::Atod(*Fields[LonIndex].TrimStartAndEnd());
			}
			if (Fields.Num() > ElevIndex)
			{
				// Parse elevation (handle "388m" or "388" or "variable")
				FString ElevStr = Fields[ElevIndex].TrimStartAndEnd();
				ElevStr.RemoveFromEnd(TEXT("m"));
				if (ElevStr == TEXT("variable"))
				{
					Location.ElevationMeters = 0.0f;
				}
				else
				{
					// Handle ranges like "200-250"
					int32 DashIndex;
					if (ElevStr.FindChar('-', DashIndex))
					{
						FString MinStr = ElevStr.Left(DashIndex);
						FString MaxStr = ElevStr.Mid(DashIndex + 1);
						float Min = FCString::Atof(*MinStr);
						float Max = FCString::Atof(*MaxStr);
						Location.ElevationMeters = (Min + Max) / 2.0f;
					}
					else
					{
						Location.ElevationMeters = FCString::Atof(*ElevStr);
					}
				}
			}

			// Parse optional fields
			int32 SignifIndex = ElevIndex + 1;
			if (Fields.Num() > SignifIndex)
			{
				Location.Significance = Fields[SignifIndex].TrimStartAndEnd();
			}
			if (Fields.Num() > SignifIndex + 1)
			{
				Location.Phase = Fields[SignifIndex + 1].TrimStartAndEnd();
			}
			if (Fields.Num() > SignifIndex + 2)
			{
				Location.DateKey = Fields[SignifIndex + 2].TrimStartAndEnd();
			}
			if (Fields.Num() > SignifIndex + 3)
			{
				Location.Notes = Fields[SignifIndex + 3].TrimStartAndEnd();
			}

			Locations.Add(Location);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Successfully loaded %d locations from CSV: %s"), Locations.Num(), *FilePath);
	return true;
}

bool UVerdunLocationDatabase::LoadFromJSON(const FString& FilePath)
{
	// Read JSON file
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load locations JSON file: %s"), *FilePath);
		return false;
	}

	// Parse JSON
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse locations JSON: %s"), *FilePath);
		return false;
	}

	// Clear existing locations
	Locations.Empty();

	// Parse locations array
	if (JsonObject->HasTypedField<EJson::Array>(TEXT("locations")))
	{
		TArray<TSharedPtr<FJsonValue>> LocationsArray = JsonObject->GetArrayField(TEXT("locations"));

		for (const TSharedPtr<FJsonValue>& LocationValue : LocationsArray)
		{
			TSharedPtr<FJsonObject> LocObj = LocationValue->AsObject();

			FVerdunLocation Location;
			Location.Name = LocObj->GetStringField(TEXT("name"));
			Location.Type = LocObj->GetStringField(TEXT("type"));
			Location.Latitude = LocObj->GetNumberField(TEXT("latitude"));
			Location.Longitude = LocObj->GetNumberField(TEXT("longitude"));
			Location.ElevationMeters = static_cast<float>(LocObj->GetNumberField(TEXT("elevation_m")));

			// Optional fields
			if (LocObj->HasField(TEXT("significance")))
			{
				Location.Significance = LocObj->GetStringField(TEXT("significance"));
			}
			if (LocObj->HasField(TEXT("phase")))
			{
				Location.Phase = LocObj->GetStringField(TEXT("phase"));
			}

			// Parse UE5 transform if present
			if (LocObj->HasTypedField<EJson::Object>(TEXT("ue5_transform")))
			{
				TSharedPtr<FJsonObject> Transform = LocObj->GetObjectField(TEXT("ue5_transform"));

				if (Transform->HasTypedField<EJson::Object>(TEXT("location")))
				{
					TSharedPtr<FJsonObject> Loc = Transform->GetObjectField(TEXT("location"));
					Location.UE5Location.X = static_cast<float>(Loc->GetNumberField(TEXT("x")));
					Location.UE5Location.Y = static_cast<float>(Loc->GetNumberField(TEXT("y")));
					Location.UE5Location.Z = static_cast<float>(Loc->GetNumberField(TEXT("z")));
				}
			}

			Locations.Add(Location);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Successfully loaded %d locations from JSON: %s"), Locations.Num(), *FilePath);
	return true;
}

TArray<FVerdunLocation> UVerdunLocationDatabase::GetLocationsByType(const FString& Type) const
{
	TArray<FVerdunLocation> Result;

	for (const FVerdunLocation& Location : Locations)
	{
		if (Location.Type.Equals(Type, ESearchCase::IgnoreCase))
		{
			Result.Add(Location);
		}
	}

	return Result;
}

FVerdunLocation UVerdunLocationDatabase::GetLocationByName(const FString& Name, bool& bFound) const
{
	for (const FVerdunLocation& Location : Locations)
	{
		if (Location.Name.Equals(Name, ESearchCase::IgnoreCase))
		{
			bFound = true;
			return Location;
		}
	}

	bFound = false;
	return FVerdunLocation();
}

TArray<FVerdunLocation> UVerdunLocationDatabase::GetFortifications() const
{
	return GetLocationsByType(TEXT("fortification"));
}

TArray<FVerdunLocation> UVerdunLocationDatabase::GetDestroyedVillages() const
{
	return GetLocationsByType(TEXT("destroyed_village"));
}

TArray<FVerdunLocation> UVerdunLocationDatabase::GetHills() const
{
	return GetLocationsByType(TEXT("hill"));
}

void UVerdunLocationDatabase::ConvertToUE5Coordinates(UVerdunTerrainParameters* TerrainParams)
{
	if (!TerrainParams)
	{
		UE_LOG(LogTemp, Error, TEXT("TerrainParams is null, cannot convert coordinates"));
		return;
	}

	for (FVerdunLocation& Location : Locations)
	{
		Location.UE5Location = TerrainParams->ConvertLatLonToUE5Coords(
			Location.Latitude,
			Location.Longitude,
			Location.ElevationMeters
		);
	}

	UE_LOG(LogTemp, Log, TEXT("Converted %d locations to UE5 coordinates"), Locations.Num());
}
