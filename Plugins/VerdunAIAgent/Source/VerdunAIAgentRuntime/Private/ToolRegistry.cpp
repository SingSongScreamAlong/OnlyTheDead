// Copyright Only The Dead Development Team. All Rights Reserved.

#include "ToolRegistry.h"
#include "TerrainSculptor.h"
#include "VerdunContentGenerator.h"
#include "LevelInspector.h"
#include "VisionSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Json.h"
#include "JsonUtilities.h"

// ============================================================================
// UToolExecutor Base Implementation
// ============================================================================

FToolExecutionResult UToolExecutor::Execute_Implementation(const TMap<FString, FString>& Parameters)
{
	FToolExecutionResult Result;
	Result.bSuccess = false;
	Result.ErrorMessage = TEXT("Tool executor not implemented");
	return Result;
}

FToolDefinition UToolExecutor::GetDefinition_Implementation() const
{
	return FToolDefinition();
}

// ============================================================================
// UToolRegistry Implementation
// ============================================================================

UToolRegistry::UToolRegistry()
{
	WorldContext = nullptr;
}

void UToolRegistry::InitializeDefaultTools(UWorld* World)
{
	WorldContext = World;

	// Register terrain tools
	UTerrainToolExecutor* TerrainExecutor = NewObject<UTerrainToolExecutor>(this);
	TerrainExecutor->World = World;
	RegisterTool(TEXT("modify_terrain"), TerrainExecutor);

	// Register content generation tools
	UContentGeneratorToolExecutor* ContentExecutor = NewObject<UContentGeneratorToolExecutor>(this);
	ContentExecutor->World = World;
	RegisterTool(TEXT("generate_content"), ContentExecutor);

	// Register level inspection tools
	ULevelInspectorToolExecutor* InspectorExecutor = NewObject<ULevelInspectorToolExecutor>(this);
	InspectorExecutor->World = World;
	RegisterTool(TEXT("inspect_level"), InspectorExecutor);

	// Register vision tools
	UVisionToolExecutor* VisionExecutor = NewObject<UVisionToolExecutor>(this);
	VisionExecutor->World = World;
	RegisterTool(TEXT("capture_and_analyze"), VisionExecutor);

	// Register actor manipulation tools
	UActorManipulatorToolExecutor* ActorExecutor = NewObject<UActorManipulatorToolExecutor>(this);
	ActorExecutor->World = World;
	RegisterTool(TEXT("manipulate_actor"), ActorExecutor);

	UE_LOG(LogTemp, Log, TEXT("ToolRegistry: Initialized with %d default tools"), RegisteredTools.Num());
}

void UToolRegistry::RegisterTool(const FString& ToolName, UToolExecutor* Executor)
{
	if (!Executor)
	{
		UE_LOG(LogTemp, Error, TEXT("ToolRegistry: Cannot register null executor for tool '%s'"), *ToolName);
		return;
	}

	RegisteredTools.Add(ToolName, Executor);
	UE_LOG(LogTemp, Log, TEXT("ToolRegistry: Registered tool '%s'"), *ToolName);
}

void UToolRegistry::UnregisterTool(const FString& ToolName)
{
	if (RegisteredTools.Remove(ToolName) > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("ToolRegistry: Unregistered tool '%s'"), *ToolName);
	}
}

TArray<FToolDefinition> UToolRegistry::GetAllToolDefinitions() const
{
	TArray<FToolDefinition> Definitions;
	for (const auto& Pair : RegisteredTools)
	{
		if (Pair.Value)
		{
			Definitions.Add(Pair.Value->GetDefinition());
		}
	}
	return Definitions;
}

TArray<FToolDefinition> UToolRegistry::GetToolsByCategory(const FString& Category) const
{
	TArray<FToolDefinition> Definitions;
	for (const auto& Pair : RegisteredTools)
	{
		if (Pair.Value)
		{
			FToolDefinition Def = Pair.Value->GetDefinition();
			if (Def.Category == Category)
			{
				Definitions.Add(Def);
			}
		}
	}
	return Definitions;
}

FToolExecutionResult UToolRegistry::ExecuteTool(const FString& ToolName, const TMap<FString, FString>& Parameters)
{
	UToolExecutor** ExecutorPtr = RegisteredTools.Find(ToolName);
	if (!ExecutorPtr || !*ExecutorPtr)
	{
		FToolExecutionResult Result;
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("Tool '%s' not found in registry"), *ToolName);
		UE_LOG(LogTemp, Error, TEXT("ToolRegistry: %s"), *Result.ErrorMessage);
		return Result;
	}

	UE_LOG(LogTemp, Log, TEXT("ToolRegistry: Executing tool '%s' with %d parameters"), *ToolName, Parameters.Num());

	FToolExecutionResult Result = (*ExecutorPtr)->Execute(Parameters);

	if (Result.bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("ToolRegistry: Tool '%s' executed successfully"), *ToolName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ToolRegistry: Tool '%s' failed: %s"), *ToolName, *Result.ErrorMessage);
	}

	return Result;
}

bool UToolRegistry::HasTool(const FString& ToolName) const
{
	return RegisteredTools.Contains(ToolName);
}

FString UToolRegistry::GenerateToolSchemaJSON() const
{
	TArray<TSharedPtr<FJsonValue>> ToolsArray;

	for (const auto& Pair : RegisteredTools)
	{
		if (!Pair.Value) continue;

		FToolDefinition Def = Pair.Value->GetDefinition();

		TSharedPtr<FJsonObject> ToolObject = MakeShareable(new FJsonObject);
		ToolObject->SetStringField(TEXT("name"), Def.Name);
		ToolObject->SetStringField(TEXT("description"), Def.Description);

		// Build input schema
		TSharedPtr<FJsonObject> InputSchema = MakeShareable(new FJsonObject);
		InputSchema->SetStringField(TEXT("type"), TEXT("object"));

		TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject);
		TArray<TSharedPtr<FJsonValue>> RequiredArray;

		for (const FToolParameterDefinition& Param : Def.Parameters)
		{
			TSharedPtr<FJsonObject> ParamObj = MakeShareable(new FJsonObject);
			ParamObj->SetStringField(TEXT("type"), Param.Type);
			ParamObj->SetStringField(TEXT("description"), Param.Description);

			if (Param.EnumValues.Num() > 0)
			{
				TArray<TSharedPtr<FJsonValue>> EnumArray;
				for (const FString& EnumVal : Param.EnumValues)
				{
					EnumArray.Add(MakeShareable(new FJsonValueString(EnumVal)));
				}
				ParamObj->SetArrayField(TEXT("enum"), EnumArray);
			}

			Properties->SetObjectField(Param.Name, ParamObj);

			if (Param.bRequired)
			{
				RequiredArray.Add(MakeShareable(new FJsonValueString(Param.Name)));
			}
		}

		InputSchema->SetObjectField(TEXT("properties"), Properties);
		InputSchema->SetArrayField(TEXT("required"), RequiredArray);

		ToolObject->SetObjectField(TEXT("input_schema"), InputSchema);

		ToolsArray.Add(MakeShareable(new FJsonValueObject(ToolObject)));
	}

	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
	RootObject->SetArrayField(TEXT("tools"), ToolsArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	return OutputString;
}

FString UToolRegistry::GenerateOpenAIFunctionsJSON() const
{
	TArray<TSharedPtr<FJsonValue>> FunctionsArray;

	for (const auto& Pair : RegisteredTools)
	{
		if (!Pair.Value) continue;

		FToolDefinition Def = Pair.Value->GetDefinition();

		TSharedPtr<FJsonObject> FunctionObject = MakeShareable(new FJsonObject);
		FunctionObject->SetStringField(TEXT("name"), Def.Name);
		FunctionObject->SetStringField(TEXT("description"), Def.Description);

		TSharedPtr<FJsonObject> Parameters = MakeShareable(new FJsonObject);
		Parameters->SetStringField(TEXT("type"), TEXT("object"));

		TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject);
		TArray<TSharedPtr<FJsonValue>> RequiredArray;

		for (const FToolParameterDefinition& Param : Def.Parameters)
		{
			TSharedPtr<FJsonObject> ParamObj = MakeShareable(new FJsonObject);
			ParamObj->SetStringField(TEXT("type"), Param.Type);
			ParamObj->SetStringField(TEXT("description"), Param.Description);

			Properties->SetObjectField(Param.Name, ParamObj);

			if (Param.bRequired)
			{
				RequiredArray.Add(MakeShareable(new FJsonValueString(Param.Name)));
			}
		}

		Parameters->SetObjectField(TEXT("properties"), Properties);
		Parameters->SetArrayField(TEXT("required"), RequiredArray);

		FunctionObject->SetObjectField(TEXT("parameters"), Parameters);

		FunctionsArray.Add(MakeShareable(new FJsonValueObject(FunctionObject)));
	}

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(FunctionsArray, Writer);

	return OutputString;
}

bool UToolRegistry::ParseToolCall(const FString& Response, FString& OutToolName, TMap<FString, FString>& OutParameters)
{
	// Parse Anthropic tool use format
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response);

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		// Check for tool_use content block
		const TArray<TSharedPtr<FJsonValue>>* ContentArray;
		if (JsonObject->TryGetArrayField(TEXT("content"), ContentArray))
		{
			for (const TSharedPtr<FJsonValue>& ContentValue : *ContentArray)
			{
				TSharedPtr<FJsonObject> ContentObj = ContentValue->AsObject();
				if (ContentObj.IsValid())
				{
					FString Type;
					if (ContentObj->TryGetStringField(TEXT("type"), Type) && Type == TEXT("tool_use"))
					{
						ContentObj->TryGetStringField(TEXT("name"), OutToolName);

						const TSharedPtr<FJsonObject>* InputObj;
						if (ContentObj->TryGetObjectField(TEXT("input"), InputObj))
						{
							for (const auto& Pair : (*InputObj)->Values)
							{
								FString Value;
								if (Pair.Value->TryGetString(Value))
								{
									OutParameters.Add(Pair.Key, Value);
								}
								else if (Pair.Value->Type == EJson::Number)
								{
									double NumValue = Pair.Value->AsNumber();
									OutParameters.Add(Pair.Key, FString::SanitizeFloat(NumValue));
								}
								else if (Pair.Value->Type == EJson::Boolean)
								{
									bool BoolValue = Pair.Value->AsBool();
									OutParameters.Add(Pair.Key, BoolValue ? TEXT("true") : TEXT("false"));
								}
							}
						}

						return true;
					}
				}
			}
		}
	}

	return false;
}

// ============================================================================
// UTerrainToolExecutor Implementation
// ============================================================================

FToolExecutionResult UTerrainToolExecutor::Execute_Implementation(const TMap<FString, FString>& Parameters)
{
	FToolExecutionResult Result;

	const FString* OperationPtr = Parameters.Find(TEXT("operation"));
	if (!OperationPtr)
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Missing required parameter: operation");
		return Result;
	}

	FString Operation = *OperationPtr;
	FString OutputResult;

	if (Operation == TEXT("raise") || Operation == TEXT("lower"))
	{
		FVector Location = FVector::ZeroVector;
		float Radius = 500.0f;
		float Strength = 1.0f;

		// Parse parameters
		if (const FString* LocStr = Parameters.Find(TEXT("location")))
		{
			TArray<FString> Components;
			LocStr->ParseIntoArray(Components, TEXT(","));
			if (Components.Num() == 3)
			{
				Location.X = FCString::Atof(*Components[0]);
				Location.Y = FCString::Atof(*Components[1]);
				Location.Z = FCString::Atof(*Components[2]);
			}
		}

		if (const FString* RadStr = Parameters.Find(TEXT("radius")))
		{
			Radius = FCString::Atof(**RadStr);
		}

		if (const FString* StrStr = Parameters.Find(TEXT("strength")))
		{
			Strength = FCString::Atof(**StrStr);
		}

		bool bSuccess = false;
		if (Operation == TEXT("raise"))
		{
			bSuccess = UTerrainSculptor::RaiseTerrain(World, Location, Radius, Strength, OutputResult);
		}
		else
		{
			bSuccess = UTerrainSculptor::LowerTerrain(World, Location, Radius, Strength, OutputResult);
		}

		Result.bSuccess = bSuccess;
		Result.Result = OutputResult;
	}
	else if (Operation == TEXT("crater"))
	{
		FCraterParams CraterParams;

		if (const FString* LocStr = Parameters.Find(TEXT("location")))
		{
			TArray<FString> Components;
			LocStr->ParseIntoArray(Components, TEXT(","));
			if (Components.Num() == 3)
			{
				CraterParams.Location.X = FCString::Atof(*Components[0]);
				CraterParams.Location.Y = FCString::Atof(*Components[1]);
				CraterParams.Location.Z = FCString::Atof(*Components[2]);
			}
		}

		if (const FString* DiamStr = Parameters.Find(TEXT("diameter")))
		{
			CraterParams.Diameter = FCString::Atof(**DiamStr);
		}

		if (const FString* DepthStr = Parameters.Find(TEXT("depth")))
		{
			CraterParams.Depth = FCString::Atof(**DepthStr);
		}

		bool bSuccess = UTerrainSculptor::CreateCrater(World, CraterParams, OutputResult);
		Result.bSuccess = bSuccess;
		Result.Result = OutputResult;
	}
	else if (Operation == TEXT("trench"))
	{
		FTrenchExcavationParams TrenchParams;

		// Parse path
		if (const FString* PathStr = Parameters.Find(TEXT("path")))
		{
			TArray<FString> Points;
			PathStr->ParseIntoArray(Points, TEXT(";"));
			for (const FString& Point : Points)
			{
				TArray<FString> Components;
				Point.ParseIntoArray(Components, TEXT(","));
				if (Components.Num() == 3)
				{
					FVector Loc;
					Loc.X = FCString::Atof(*Components[0]);
					Loc.Y = FCString::Atof(*Components[1]);
					Loc.Z = FCString::Atof(*Components[2]);
					TrenchParams.Path.Add(Loc);
				}
			}
		}

		if (const FString* WidthStr = Parameters.Find(TEXT("width")))
		{
			TrenchParams.Width = FCString::Atof(**WidthStr);
		}

		if (const FString* DepthStr = Parameters.Find(TEXT("depth")))
		{
			TrenchParams.Depth = FCString::Atof(**DepthStr);
		}

		bool bSuccess = UTerrainSculptor::ExcavateTrench(World, TrenchParams, OutputResult);
		Result.bSuccess = bSuccess;
		Result.Result = OutputResult;
	}
	else
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("Unknown terrain operation: %s"), *Operation);
	}

	return Result;
}

FToolDefinition UTerrainToolExecutor::GetDefinition_Implementation() const
{
	FToolDefinition Def;
	Def.Name = TEXT("modify_terrain");
	Def.Description = TEXT("Modify terrain landscape by raising, lowering, creating craters, or excavating trenches");
	Def.Category = TEXT("terrain");

	FToolParameterDefinition OpParam;
	OpParam.Name = TEXT("operation");
	OpParam.Type = TEXT("string");
	OpParam.Description = TEXT("Type of terrain operation to perform");
	OpParam.bRequired = true;
	OpParam.EnumValues = {TEXT("raise"), TEXT("lower"), TEXT("crater"), TEXT("trench")};
	Def.Parameters.Add(OpParam);

	FToolParameterDefinition LocParam;
	LocParam.Name = TEXT("location");
	LocParam.Type = TEXT("string");
	LocParam.Description = TEXT("Location in world space (format: 'x,y,z')");
	LocParam.bRequired = true;
	Def.Parameters.Add(LocParam);

	FToolParameterDefinition RadParam;
	RadParam.Name = TEXT("radius");
	RadParam.Type = TEXT("number");
	RadParam.Description = TEXT("Radius of effect in UE units (default: 500)");
	RadParam.bRequired = false;
	Def.Parameters.Add(RadParam);

	FToolParameterDefinition StrParam;
	StrParam.Name = TEXT("strength");
	StrParam.Type = TEXT("number");
	StrParam.Description = TEXT("Strength of operation 0-1 (default: 1.0)");
	StrParam.bRequired = false;
	Def.Parameters.Add(StrParam);

	return Def;
}

// ============================================================================
// UContentGeneratorToolExecutor Implementation
// ============================================================================

FToolExecutionResult UContentGeneratorToolExecutor::Execute_Implementation(const TMap<FString, FString>& Parameters)
{
	FToolExecutionResult Result;

	const FString* TypePtr = Parameters.Find(TEXT("content_type"));
	if (!TypePtr)
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Missing required parameter: content_type");
		return Result;
	}

	FString ContentType = *TypePtr;
	FString OutputResult;

	if (ContentType == TEXT("trench_system"))
	{
		TArray<FVector> Path;

		// Parse path
		if (const FString* PathStr = Parameters.Find(TEXT("path")))
		{
			TArray<FString> Points;
			PathStr->ParseIntoArray(Points, TEXT(";"));
			for (const FString& Point : Points)
			{
				TArray<FString> Components;
				Point.ParseIntoArray(Components, TEXT(","));
				if (Components.Num() == 3)
				{
					FVector Loc;
					Loc.X = FCString::Atof(*Components[0]);
					Loc.Y = FCString::Atof(*Components[1]);
					Loc.Z = FCString::Atof(*Components[2]);
					Path.Add(Loc);
				}
			}
		}

		FTrenchConfiguration Config;
		if (const FString* WidthStr = Parameters.Find(TEXT("width")))
		{
			Config.Width = FCString::Atof(**WidthStr);
		}

		if (const FString* DepthStr = Parameters.Find(TEXT("depth")))
		{
			Config.Depth = FCString::Atof(**DepthStr);
		}

		UVerdunContentGenerator::GenerateTrenchSystem(World, Path, Config);

		Result.bSuccess = true;
		Result.Result = FString::Printf(TEXT("Generated trench system with %d segments"), Path.Num() - 1);
	}
	else if (ContentType == TEXT("barbed_wire"))
	{
		TArray<FVector> Path;

		if (const FString* PathStr = Parameters.Find(TEXT("path")))
		{
			TArray<FString> Points;
			PathStr->ParseIntoArray(Points, TEXT(";"));
			for (const FString& Point : Points)
			{
				TArray<FString> Components;
				Point.ParseIntoArray(Components, TEXT(","));
				if (Components.Num() == 3)
				{
					FVector Loc;
					Loc.X = FCString::Atof(*Components[0]);
					Loc.Y = FCString::Atof(*Components[1]);
					Loc.Z = FCString::Atof(*Components[2]);
					Path.Add(Loc);
				}
			}
		}

		int32 Rows = 3;
		if (const FString* RowsStr = Parameters.Find(TEXT("rows")))
		{
			Rows = FCString::Atoi(**RowsStr);
		}

		float Height = 100.0f;
		if (const FString* HeightStr = Parameters.Find(TEXT("height")))
		{
			Height = FCString::Atof(**HeightStr);
		}

		UVerdunContentGenerator::GenerateBarbedWire(World, Path, Rows, Height);

		Result.bSuccess = true;
		Result.Result = FString::Printf(TEXT("Generated barbed wire obstacle with %d rows"), Rows);
	}
	else
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("Unknown content type: %s"), *ContentType);
	}

	return Result;
}

FToolDefinition UContentGeneratorToolExecutor::GetDefinition_Implementation() const
{
	FToolDefinition Def;
	Def.Name = TEXT("generate_content");
	Def.Description = TEXT("Generate WWI battlefield content like trenches, barbed wire, and fortifications");
	Def.Category = TEXT("content");

	FToolParameterDefinition TypeParam;
	TypeParam.Name = TEXT("content_type");
	TypeParam.Type = TEXT("string");
	TypeParam.Description = TEXT("Type of content to generate");
	TypeParam.bRequired = true;
	TypeParam.EnumValues = {TEXT("trench_system"), TEXT("barbed_wire"), TEXT("dugout"), TEXT("crater_field")};
	Def.Parameters.Add(TypeParam);

	FToolParameterDefinition PathParam;
	PathParam.Name = TEXT("path");
	PathParam.Type = TEXT("string");
	PathParam.Description = TEXT("Path points (format: 'x1,y1,z1;x2,y2,z2;...')");
	PathParam.bRequired = true;
	Def.Parameters.Add(PathParam);

	FToolParameterDefinition WidthParam;
	WidthParam.Name = TEXT("width");
	WidthParam.Type = TEXT("number");
	WidthParam.Description = TEXT("Width in UE units (default: 150)");
	WidthParam.bRequired = false;
	Def.Parameters.Add(WidthParam);

	FToolParameterDefinition DepthParam;
	DepthParam.Name = TEXT("depth");
	DepthParam.Type = TEXT("number");
	DepthParam.Description = TEXT("Depth in UE units (default: 200)");
	DepthParam.bRequired = false;
	Def.Parameters.Add(DepthParam);

	return Def;
}

// ============================================================================
// ULevelInspectorToolExecutor Implementation
// ============================================================================

FToolExecutionResult ULevelInspectorToolExecutor::Execute_Implementation(const TMap<FString, FString>& Parameters)
{
	FToolExecutionResult Result;

	const FString* QueryPtr = Parameters.Find(TEXT("query_type"));
	if (!QueryPtr)
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Missing required parameter: query_type");
		return Result;
	}

	FString QueryType = *QueryPtr;

	if (QueryType == TEXT("summary"))
	{
		FString Summary = ULevelInspector::GetLevelSummaryText(World);
		Result.bSuccess = true;
		Result.Result = Summary;
	}
	else if (QueryType == TEXT("find_actors"))
	{
		const FString* NamePattern = Parameters.Find(TEXT("name_pattern"));
		if (NamePattern)
		{
			TArray<AActor*> Actors = ULevelInspector::FindActorsByName(World, *NamePattern);
			Result.bSuccess = true;
			Result.Result = FString::Printf(TEXT("Found %d actors matching pattern '%s'"), Actors.Num(), **NamePattern);
			Result.OutputData.Add(TEXT("actor_count"), FString::FromInt(Actors.Num()));
		}
		else
		{
			Result.bSuccess = false;
			Result.ErrorMessage = TEXT("Missing name_pattern parameter");
		}
	}
	else if (QueryType == TEXT("actors_in_radius"))
	{
		FVector Center = FVector::ZeroVector;
		float Radius = 1000.0f;

		if (const FString* CenterStr = Parameters.Find(TEXT("center")))
		{
			TArray<FString> Components;
			CenterStr->ParseIntoArray(Components, TEXT(","));
			if (Components.Num() == 3)
			{
				Center.X = FCString::Atof(*Components[0]);
				Center.Y = FCString::Atof(*Components[1]);
				Center.Z = FCString::Atof(*Components[2]);
			}
		}

		if (const FString* RadStr = Parameters.Find(TEXT("radius")))
		{
			Radius = FCString::Atof(**RadStr);
		}

		TArray<AActor*> Actors = ULevelInspector::GetActorsInRadius(World, Center, Radius);
		Result.bSuccess = true;
		Result.Result = FString::Printf(TEXT("Found %d actors within radius %.0f"), Actors.Num(), Radius);
		Result.OutputData.Add(TEXT("actor_count"), FString::FromInt(Actors.Num()));
	}
	else if (QueryType == TEXT("ai_context"))
	{
		FString Context = ULevelInspector::GenerateAIContextString(World);
		Result.bSuccess = true;
		Result.Result = Context;
	}
	else
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("Unknown query type: %s"), *QueryType);
	}

	return Result;
}

FToolDefinition ULevelInspectorToolExecutor::GetDefinition_Implementation() const
{
	FToolDefinition Def;
	Def.Name = TEXT("inspect_level");
	Def.Description = TEXT("Query and inspect the current level state, actors, and scene information");
	Def.Category = TEXT("inspection");

	FToolParameterDefinition QueryParam;
	QueryParam.Name = TEXT("query_type");
	QueryParam.Type = TEXT("string");
	QueryParam.Description = TEXT("Type of inspection query to perform");
	QueryParam.bRequired = true;
	QueryParam.EnumValues = {TEXT("summary"), TEXT("find_actors"), TEXT("actors_in_radius"), TEXT("ai_context")};
	Def.Parameters.Add(QueryParam);

	FToolParameterDefinition NameParam;
	NameParam.Name = TEXT("name_pattern");
	NameParam.Type = TEXT("string");
	NameParam.Description = TEXT("Name pattern for actor search (supports wildcards)");
	NameParam.bRequired = false;
	Def.Parameters.Add(NameParam);

	FToolParameterDefinition CenterParam;
	CenterParam.Name = TEXT("center");
	CenterParam.Type = TEXT("string");
	CenterParam.Description = TEXT("Center position for radius search (format: 'x,y,z')");
	CenterParam.bRequired = false;
	Def.Parameters.Add(CenterParam);

	FToolParameterDefinition RadiusParam;
	RadiusParam.Name = TEXT("radius");
	RadiusParam.Type = TEXT("number");
	RadiusParam.Description = TEXT("Search radius in UE units");
	RadiusParam.bRequired = false;
	Def.Parameters.Add(RadiusParam);

	return Def;
}

// ============================================================================
// UVisionToolExecutor Implementation
// ============================================================================

FToolExecutionResult UVisionToolExecutor::Execute_Implementation(const TMap<FString, FString>& Parameters)
{
	FToolExecutionResult Result;

	const FString* ActionPtr = Parameters.Find(TEXT("action"));
	if (!ActionPtr)
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Missing required parameter: action");
		return Result;
	}

	FString Action = *ActionPtr;

	if (Action == TEXT("capture_viewport"))
	{
		FScreenshotSettings Settings;

		if (const FString* WidthStr = Parameters.Find(TEXT("width")))
		{
			Settings.Width = FCString::Atoi(**WidthStr);
		}

		if (const FString* HeightStr = Parameters.Find(TEXT("height")))
		{
			Settings.Height = FCString::Atoi(**HeightStr);
		}

		FString ScreenshotPath = UVisionSystem::CaptureViewportScreenshot(Settings);

		if (!ScreenshotPath.IsEmpty())
		{
			Result.bSuccess = true;
			Result.Result = FString::Printf(TEXT("Screenshot captured: %s"), *ScreenshotPath);
			Result.OutputData.Add(TEXT("screenshot_path"), ScreenshotPath);
		}
		else
		{
			Result.bSuccess = false;
			Result.ErrorMessage = TEXT("Failed to capture screenshot");
		}
	}
	else if (Action == TEXT("capture_from_position"))
	{
		FVector Position = FVector::ZeroVector;
		FRotator Rotation = FRotator::ZeroRotator;
		FScreenshotSettings Settings;

		if (const FString* PosStr = Parameters.Find(TEXT("position")))
		{
			TArray<FString> Components;
			PosStr->ParseIntoArray(Components, TEXT(","));
			if (Components.Num() == 3)
			{
				Position.X = FCString::Atof(*Components[0]);
				Position.Y = FCString::Atof(*Components[1]);
				Position.Z = FCString::Atof(*Components[2]);
			}
		}

		if (const FString* RotStr = Parameters.Find(TEXT("rotation")))
		{
			TArray<FString> Components;
			RotStr->ParseIntoArray(Components, TEXT(","));
			if (Components.Num() == 3)
			{
				Rotation.Pitch = FCString::Atof(*Components[0]);
				Rotation.Yaw = FCString::Atof(*Components[1]);
				Rotation.Roll = FCString::Atof(*Components[2]);
			}
		}

		FString ScreenshotPath = UVisionSystem::CaptureFromPosition(Position, Rotation, Settings);

		if (!ScreenshotPath.IsEmpty())
		{
			Result.bSuccess = true;
			Result.Result = FString::Printf(TEXT("Screenshot captured from position: %s"), *ScreenshotPath);
			Result.OutputData.Add(TEXT("screenshot_path"), ScreenshotPath);
		}
		else
		{
			Result.bSuccess = false;
			Result.ErrorMessage = TEXT("Failed to capture screenshot");
		}
	}
	else
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("Unknown vision action: %s"), *Action);
	}

	return Result;
}

FToolDefinition UVisionToolExecutor::GetDefinition_Implementation() const
{
	FToolDefinition Def;
	Def.Name = TEXT("capture_and_analyze");
	Def.Description = TEXT("Capture screenshots and analyze visual content using AI vision");
	Def.Category = TEXT("vision");

	FToolParameterDefinition ActionParam;
	ActionParam.Name = TEXT("action");
	ActionParam.Type = TEXT("string");
	ActionParam.Description = TEXT("Vision action to perform");
	ActionParam.bRequired = true;
	ActionParam.EnumValues = {TEXT("capture_viewport"), TEXT("capture_from_position"), TEXT("analyze_image")};
	Def.Parameters.Add(ActionParam);

	FToolParameterDefinition PosParam;
	PosParam.Name = TEXT("position");
	PosParam.Type = TEXT("string");
	PosParam.Description = TEXT("Camera position (format: 'x,y,z')");
	PosParam.bRequired = false;
	Def.Parameters.Add(PosParam);

	FToolParameterDefinition RotParam;
	RotParam.Name = TEXT("rotation");
	RotParam.Type = TEXT("string");
	RotParam.Description = TEXT("Camera rotation (format: 'pitch,yaw,roll')");
	RotParam.bRequired = false;
	Def.Parameters.Add(RotParam);

	return Def;
}

// ============================================================================
// UActorManipulatorToolExecutor Implementation
// ============================================================================

FToolExecutionResult UActorManipulatorToolExecutor::Execute_Implementation(const TMap<FString, FString>& Parameters)
{
	FToolExecutionResult Result;

	const FString* ActionPtr = Parameters.Find(TEXT("action"));
	if (!ActionPtr)
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Missing required parameter: action");
		return Result;
	}

	FString Action = *ActionPtr;

	if (Action == TEXT("spawn"))
	{
		const FString* ClassPath = Parameters.Find(TEXT("class_path"));
		if (!ClassPath)
		{
			Result.bSuccess = false;
			Result.ErrorMessage = TEXT("Missing class_path parameter");
			return Result;
		}

		FVector Location = FVector::ZeroVector;
		FRotator Rotation = FRotator::ZeroRotator;

		if (const FString* LocStr = Parameters.Find(TEXT("location")))
		{
			TArray<FString> Components;
			LocStr->ParseIntoArray(Components, TEXT(","));
			if (Components.Num() == 3)
			{
				Location.X = FCString::Atof(*Components[0]);
				Location.Y = FCString::Atof(*Components[1]);
				Location.Z = FCString::Atof(*Components[2]);
			}
		}

		if (const FString* RotStr = Parameters.Find(TEXT("rotation")))
		{
			TArray<FString> Components;
			RotStr->ParseIntoArray(Components, TEXT(","));
			if (Components.Num() == 3)
			{
				Rotation.Pitch = FCString::Atof(*Components[0]);
				Rotation.Yaw = FCString::Atof(*Components[1]);
				Rotation.Roll = FCString::Atof(*Components[2]);
			}
		}

		UClass* ActorClass = LoadClass<AActor>(nullptr, **ClassPath);
		if (ActorClass)
		{
			AActor* NewActor = World->SpawnActor<AActor>(ActorClass, Location, Rotation);
			if (NewActor)
			{
				Result.bSuccess = true;
				Result.Result = FString::Printf(TEXT("Spawned actor: %s"), *NewActor->GetName());
				Result.OutputData.Add(TEXT("actor_name"), NewActor->GetName());
			}
			else
			{
				Result.bSuccess = false;
				Result.ErrorMessage = TEXT("Failed to spawn actor");
			}
		}
		else
		{
			Result.bSuccess = false;
			Result.ErrorMessage = FString::Printf(TEXT("Failed to load class: %s"), **ClassPath);
		}
	}
	else if (Action == TEXT("move"))
	{
		const FString* ActorName = Parameters.Find(TEXT("actor_name"));
		if (!ActorName)
		{
			Result.bSuccess = false;
			Result.ErrorMessage = TEXT("Missing actor_name parameter");
			return Result;
		}

		TArray<AActor*> Actors = ULevelInspector::FindActorsByName(World, *ActorName);
		if (Actors.Num() > 0)
		{
			AActor* Actor = Actors[0];

			FVector NewLocation = Actor->GetActorLocation();
			if (const FString* LocStr = Parameters.Find(TEXT("location")))
			{
				TArray<FString> Components;
				LocStr->ParseIntoArray(Components, TEXT(","));
				if (Components.Num() == 3)
				{
					NewLocation.X = FCString::Atof(*Components[0]);
					NewLocation.Y = FCString::Atof(*Components[1]);
					NewLocation.Z = FCString::Atof(*Components[2]);
				}
			}

			Actor->SetActorLocation(NewLocation);

			Result.bSuccess = true;
			Result.Result = FString::Printf(TEXT("Moved actor '%s' to %s"), *Actor->GetName(), *NewLocation.ToString());
		}
		else
		{
			Result.bSuccess = false;
			Result.ErrorMessage = FString::Printf(TEXT("Actor not found: %s"), **ActorName);
		}
	}
	else if (Action == TEXT("delete"))
	{
		const FString* ActorName = Parameters.Find(TEXT("actor_name"));
		if (!ActorName)
		{
			Result.bSuccess = false;
			Result.ErrorMessage = TEXT("Missing actor_name parameter");
			return Result;
		}

		TArray<AActor*> Actors = ULevelInspector::FindActorsByName(World, *ActorName);
		if (Actors.Num() > 0)
		{
			AActor* Actor = Actors[0];
			FString Name = Actor->GetName();
			Actor->Destroy();

			Result.bSuccess = true;
			Result.Result = FString::Printf(TEXT("Deleted actor: %s"), *Name);
		}
		else
		{
			Result.bSuccess = false;
			Result.ErrorMessage = FString::Printf(TEXT("Actor not found: %s"), **ActorName);
		}
	}
	else
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("Unknown actor action: %s"), *Action);
	}

	return Result;
}

FToolDefinition UActorManipulatorToolExecutor::GetDefinition_Implementation() const
{
	FToolDefinition Def;
	Def.Name = TEXT("manipulate_actor");
	Def.Description = TEXT("Spawn, move, or delete actors in the level");
	Def.Category = TEXT("actor");

	FToolParameterDefinition ActionParam;
	ActionParam.Name = TEXT("action");
	ActionParam.Type = TEXT("string");
	ActionParam.Description = TEXT("Action to perform on actor");
	ActionParam.bRequired = true;
	ActionParam.EnumValues = {TEXT("spawn"), TEXT("move"), TEXT("delete"), TEXT("rotate"), TEXT("scale")};
	Def.Parameters.Add(ActionParam);

	FToolParameterDefinition ClassParam;
	ClassParam.Name = TEXT("class_path");
	ClassParam.Type = TEXT("string");
	ClassParam.Description = TEXT("Full class path for spawning (e.g., '/Script/Engine.StaticMeshActor')");
	ClassParam.bRequired = false;
	Def.Parameters.Add(ClassParam);

	FToolParameterDefinition NameParam;
	NameParam.Name = TEXT("actor_name");
	NameParam.Type = TEXT("string");
	NameParam.Description = TEXT("Name of actor to manipulate");
	NameParam.bRequired = false;
	Def.Parameters.Add(NameParam);

	FToolParameterDefinition LocParam;
	LocParam.Name = TEXT("location");
	LocParam.Type = TEXT("string");
	LocParam.Description = TEXT("Location (format: 'x,y,z')");
	LocParam.bRequired = false;
	Def.Parameters.Add(LocParam);

	FToolParameterDefinition RotParam;
	RotParam.Name = TEXT("rotation");
	RotParam.Type = TEXT("string");
	RotParam.Description = TEXT("Rotation (format: 'pitch,yaw,roll')");
	RotParam.bRequired = false;
	Def.Parameters.Add(RotParam);

	return Def;
}
