// Copyright Epic Games, Inc. All Rights Reserved.

#include "EditorCommandExecutor.h"
#include "ClaudeTerminalSettings.h"
#include "JsonUtilities.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"
#include "Editor.h"

FEditorCommandExecutor::FEditorCommandExecutor()
{
}

FEditorCommandExecutor::~FEditorCommandExecutor()
{
}

bool FEditorCommandExecutor::ProcessResponse(const FString& Response, UWorld* World, FString& OutExecutionLog)
{
	if (!World)
	{
		OutExecutionLog = TEXT("Error: No valid world context.");
		return false;
	}

	// Try to extract JSON command
	TSharedPtr<FJsonObject> Command;
	if (!ExtractJSONCommand(Response, Command))
	{
		// No command found, just conversational
		return false;
	}

	// Get command type
	FString CommandType;
	if (!Command->TryGetStringField(TEXT("command"), CommandType))
	{
		OutExecutionLog = TEXT("Error: Command JSON missing 'command' field.");
		return false;
	}

	// Get parameters
	const TSharedPtr<FJsonObject>* ParametersPtr;
	if (!Command->TryGetObjectField(TEXT("parameters"), ParametersPtr))
	{
		OutExecutionLog = TEXT("Error: Command JSON missing 'parameters' field.");
		return false;
	}
	TSharedPtr<FJsonObject> Parameters = *ParametersPtr;

	// Check if undo is enabled
	const UClaudeTerminalSettings* Settings = GetDefault<UClaudeTerminalSettings>();
	bool bEnableUndo = Settings ? Settings->bEnableUndo : true;

	// Begin transaction if enabled
	FScopedTransaction* Transaction = nullptr;
	if (bEnableUndo && GEditor)
	{
		Transaction = new FScopedTransaction(FText::FromString(FString::Printf(TEXT("Claude Command: %s"), *CommandType)));
	}

	// Route to appropriate handler
	bool bSuccess = false;

	// Try landscape commands first
	if (CommandType == TEXT("create_landscape"))
	{
		bSuccess = ExecuteCreateLandscape(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("import_terrain"))
	{
		bSuccess = ExecuteImportTerrain(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("modify_terrain"))
	{
		bSuccess = ExecuteModifyTerrain(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("create_crater"))
	{
		bSuccess = ExecuteCreateCrater(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("create_trench"))
	{
		bSuccess = ExecuteCreateTrench(Parameters, World, OutExecutionLog);
	}
	else
	{
		// Fall back to base executor for standard commands
		if (Transaction)
		{
			delete Transaction;
			Transaction = nullptr;
		}
		return BaseExecutor.ProcessResponse(Response, World, OutExecutionLog);
	}

	// Clean up transaction
	if (Transaction)
	{
		delete Transaction;
	}

	return bSuccess;
}

bool FEditorCommandExecutor::ExtractJSONCommand(const FString& Response, TSharedPtr<FJsonObject>& OutCommand)
{
	// Same logic as base executor
	int32 StartIdx = Response.Find(TEXT("```json"));
	if (StartIdx == INDEX_NONE)
	{
		StartIdx = Response.Find(TEXT("```JSON"));
	}
	if (StartIdx == INDEX_NONE)
	{
		StartIdx = Response.Find(TEXT("{"));
		if (StartIdx == INDEX_NONE)
		{
			return false;
		}
	}
	else
	{
		StartIdx = Response.Find(TEXT("\n"), StartIdx);
		if (StartIdx == INDEX_NONE)
		{
			return false;
		}
		StartIdx++;
	}

	int32 EndIdx = Response.Find(TEXT("```"), StartIdx);
	if (EndIdx == INDEX_NONE)
	{
		int32 BraceCount = 0;
		EndIdx = StartIdx;
		for (int32 i = StartIdx; i < Response.Len(); i++)
		{
			if (Response[i] == TEXT('{'))
			{
				BraceCount++;
			}
			else if (Response[i] == TEXT('}'))
			{
				BraceCount--;
				if (BraceCount == 0)
				{
					EndIdx = i + 1;
					break;
				}
			}
		}
	}

	if (EndIdx <= StartIdx)
	{
		return false;
	}

	FString JSONString = Response.Mid(StartIdx, EndIdx - StartIdx).TrimStartAndEnd();

	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JSONString);
	if (!FJsonSerializer::Deserialize(Reader, OutCommand) || !OutCommand.IsValid())
	{
		return false;
	}

	return true;
}

bool FEditorCommandExecutor::ExecuteCreateLandscape(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	FString HeightmapPath;
	if (!Parameters->TryGetStringField(TEXT("heightmap_path"), HeightmapPath))
	{
		OutLog = TEXT("Error: create_landscape requires 'heightmap_path' parameter.");
		return false;
	}

	FVector Location = ParseVector(Parameters, TEXT("location"), FVector::ZeroVector);
	FVector Scale = ParseVector(Parameters, TEXT("scale"), FVector(100.0, 100.0, 100.0));

	// Optional material
	FString MaterialPath;
	UMaterialInterface* Material = nullptr;
	if (Parameters->TryGetStringField(TEXT("material_path"), MaterialPath))
	{
		Material = LoadObject<UMaterialInterface>(nullptr, *MaterialPath);
	}

	// Create landscape
	ALandscape* Landscape = LandscapeManager.CreateLandscapeFromHeightmap(
		World, HeightmapPath, Location, Scale, Material, OutLog);

	if (Landscape && GEditor)
	{
		GEditor->SelectNone(false, true);
		GEditor->SelectActor(Landscape, true, true);
		GEditor->NoteSelectionChange();
		OutLog += TEXT("\nLandscape created and selected.");
	}

	return Landscape != nullptr;
}

bool FEditorCommandExecutor::ExecuteImportTerrain(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	FString HeightmapPath;
	if (!Parameters->TryGetStringField(TEXT("heightmap_path"), HeightmapPath))
	{
		OutLog = TEXT("Error: import_terrain requires 'heightmap_path' parameter.");
		return false;
	}

	double GPSLat, GPSLon, MetersPerPixel;
	if (!Parameters->TryGetNumberField(TEXT("gps_latitude"), GPSLat) ||
	    !Parameters->TryGetNumberField(TEXT("gps_longitude"), GPSLon))
	{
		OutLog = TEXT("Error: import_terrain requires 'gps_latitude' and 'gps_longitude' parameters.");
		return false;
	}

	// Optional: meters per pixel (default to 10m SRTM resolution)
	MetersPerPixel = 10.0;
	Parameters->TryGetNumberField(TEXT("meters_per_pixel"), MetersPerPixel);

	// Optional material
	FString MaterialPath;
	UMaterialInterface* Material = nullptr;
	if (Parameters->TryGetStringField(TEXT("material_path"), MaterialPath))
	{
		Material = LoadObject<UMaterialInterface>(nullptr, *MaterialPath);
	}

	// Create landscape from GPS data
	ALandscape* Landscape = LandscapeManager.CreateLandscapeFromGPSData(
		World, HeightmapPath, GPSLat, GPSLon, MetersPerPixel, Material, OutLog);

	if (Landscape && GEditor)
	{
		GEditor->SelectNone(false, true);
		GEditor->SelectActor(Landscape, true, true);
		GEditor->NoteSelectionChange();
		OutLog += TEXT("\nTerrain imported and selected.");
	}

	return Landscape != nullptr;
}

bool FEditorCommandExecutor::ExecuteModifyTerrain(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	FString Operation;
	if (!Parameters->TryGetStringField(TEXT("operation"), Operation))
	{
		OutLog = TEXT("Error: modify_terrain requires 'operation' parameter (raise|lower|smooth|flatten).");
		return false;
	}

	FVector Location = ParseVector(Parameters, TEXT("location"));

	double Radius = 1000.0;
	double Strength = 0.5;
	Parameters->TryGetNumberField(TEXT("radius"), Radius);
	Parameters->TryGetNumberField(TEXT("strength"), Strength);

	return LandscapeManager.ModifyLandscapeTerrain(
		World, Location, static_cast<float>(Radius), static_cast<float>(Strength), Operation, OutLog);
}

bool FEditorCommandExecutor::ExecuteCreateCrater(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	FVector Location = ParseVector(Parameters, TEXT("location"));

	double Radius = 500.0;  // 5 meters default
	double Depth = 200.0;   // 2 meters default
	Parameters->TryGetNumberField(TEXT("radius"), Radius);
	Parameters->TryGetNumberField(TEXT("depth"), Depth);

	return LandscapeManager.CreateCrater(
		World, Location, static_cast<float>(Radius), static_cast<float>(Depth), OutLog);
}

bool FEditorCommandExecutor::ExecuteCreateTrench(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	FVector StartLocation = ParseVector(Parameters, TEXT("start_location"));
	FVector EndLocation = ParseVector(Parameters, TEXT("end_location"));

	double Width = 200.0;   // 2 meters default
	double Depth = 250.0;   // 2.5 meters default
	Parameters->TryGetNumberField(TEXT("width"), Width);
	Parameters->TryGetNumberField(TEXT("depth"), Depth);

	return LandscapeManager.CreateTrench(
		World, StartLocation, EndLocation, static_cast<float>(Width), static_cast<float>(Depth), OutLog);
}

FVector FEditorCommandExecutor::ParseVector(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, const FVector& Default)
{
	const TSharedPtr<FJsonObject>* VectorObj;
	if (!Obj->TryGetObjectField(FieldName, VectorObj))
	{
		return Default;
	}

	FVector Result = Default;
	(*VectorObj)->TryGetNumberField(TEXT("x"), Result.X);
	(*VectorObj)->TryGetNumberField(TEXT("y"), Result.Y);
	(*VectorObj)->TryGetNumberField(TEXT("z"), Result.Z);

	return Result;
}

FRotator FEditorCommandExecutor::ParseRotator(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, const FRotator& Default)
{
	const TSharedPtr<FJsonObject>* RotatorObj;
	if (!Obj->TryGetObjectField(FieldName, RotatorObj))
	{
		return Default;
	}

	FRotator Result = Default;
	(*RotatorObj)->TryGetNumberField(TEXT("pitch"), Result.Pitch);
	(*RotatorObj)->TryGetNumberField(TEXT("yaw"), Result.Yaw);
	(*RotatorObj)->TryGetNumberField(TEXT("roll"), Result.Roll);

	return Result;
}

FString FEditorCommandExecutor::GetHelpText()
{
	FString BaseHelp = FCommandExecutor::GetHelpText();

	FString LandscapeHelp = TEXT(
		"\n"
		"=== LANDSCAPE & TERRAIN COMMANDS ===\n\n"

		"6. create_landscape - Create landscape from heightmap file\n"
		"   Examples:\n"
		"   - 'Create a landscape from my heightmap at 0,0,0'\n"
		"   - 'Load terrain from verdun_srtm.r16'\n"
		"   - 'Import heightmap with 10m per pixel scale'\n\n"

		"7. import_terrain - Import GPS-referenced terrain (SRTM data)\n"
		"   Examples:\n"
		"   - 'Import Verdun terrain from SRTM data'\n"
		"   - 'Load battlefield at GPS 49.1597°N, 5.4267°E'\n"
		"   - 'Create landscape from Fort Douaumont coordinates'\n\n"

		"8. modify_terrain - Modify existing landscape\n"
		"   Examples:\n"
		"   - 'Raise terrain by 5 meters at 1000,500,0'\n"
		"   - 'Flatten the ground here with radius 100m'\n"
		"   - 'Smooth the landscape around this location'\n\n"

		"9. create_crater - Create shell crater in terrain\n"
		"   Examples:\n"
		"   - 'Create a 10-meter crater at 2000,1000,0'\n"
		"   - 'Add shell crater with 5m radius and 2m depth'\n"
		"   - 'Make explosion crater here'\n\n"

		"10. create_trench - Carve trench line in terrain\n"
		"   Examples:\n"
		"   - 'Create trench from 0,0,0 to 5000,0,0'\n"
		"   - 'Dig 2-meter deep trench along this line'\n"
		"   - 'Build WWI trench system connecting these points'\n\n"

		"VERDUN-SPECIFIC FEATURES:\n"
		"- Import real SRTM elevation data\n"
		"- GPS coordinates automatically converted to UE5\n"
		"- Create historically accurate trenches and craters\n"
		"- Real-world scale (meters) preserved\n\n"

		"TIP: Your Verdun data in verdun_anthology/geographic_data/ can be\n"
		"     loaded directly using import_terrain with GPS coordinates!\n"
		"================================\n"
	);

	return BaseHelp + LandscapeHelp;
}
