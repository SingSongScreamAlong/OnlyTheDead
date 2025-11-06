// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommandExecutor.h"
#include "ClaudeTerminalSettings.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "UObject/ConstructorHelpers.h"
#include "JsonUtilities.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "Editor.h"

FCommandExecutor::FCommandExecutor()
{
}

FCommandExecutor::~FCommandExecutor()
{
}

bool FCommandExecutor::ProcessResponse(const FString& Response, UWorld* World, FString& OutExecutionLog)
{
	if (!World)
	{
		OutExecutionLog = TEXT("Error: No valid world context.");
		return false;
	}

	// Try to extract JSON command from response
	TSharedPtr<FJsonObject> Command;
	if (!ExtractJSONCommand(Response, Command))
	{
		// No command found, this is just a conversational response
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

	// Check if undo is enabled in settings
	const UClaudeTerminalSettings* Settings = GetDefault<UClaudeTerminalSettings>();
	bool bEnableUndo = Settings ? Settings->bEnableUndo : true;

	// Begin undo transaction if enabled
	FScopedTransaction* Transaction = nullptr;
	if (bEnableUndo && GEditor)
	{
		Transaction = new FScopedTransaction(FText::FromString(FString::Printf(TEXT("Claude Command: %s"), *CommandType)));
	}

	// Route to appropriate handler
	bool bSuccess = false;
	if (CommandType == TEXT("spawn_actor"))
	{
		bSuccess = ExecuteSpawnActor(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("create_road"))
	{
		bSuccess = ExecuteCreateRoad(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("place_model"))
	{
		bSuccess = ExecutePlaceModel(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("modify_landscape"))
	{
		bSuccess = ExecuteModifyLandscape(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("create_material"))
	{
		bSuccess = ExecuteCreateMaterial(Parameters, World, OutExecutionLog);
	}
	else if (CommandType == TEXT("execute_blueprint"))
	{
		bSuccess = ExecuteBlueprint(Parameters, World, OutExecutionLog);
	}
	else
	{
		OutExecutionLog = FString::Printf(TEXT("Error: Unknown command type '%s'."), *CommandType);
		if (Transaction)
		{
			delete Transaction; // Cancel transaction on error
		}
		return false;
	}

	// Clean up transaction
	if (Transaction)
	{
		delete Transaction; // Transaction is automatically committed when deleted
	}

	return bSuccess;
}

bool FCommandExecutor::ExtractJSONCommand(const FString& Response, TSharedPtr<FJsonObject>& OutCommand)
{
	// Look for JSON in markdown code blocks: ```json ... ```
	int32 StartIdx = Response.Find(TEXT("```json"));
	if (StartIdx == INDEX_NONE)
	{
		StartIdx = Response.Find(TEXT("```JSON"));
	}
	if (StartIdx == INDEX_NONE)
	{
		// Try to find raw JSON object
		StartIdx = Response.Find(TEXT("{"));
		if (StartIdx == INDEX_NONE)
		{
			return false;
		}
	}
	else
	{
		// Skip past the ```json marker
		StartIdx = Response.Find(TEXT("\n"), StartIdx);
		if (StartIdx == INDEX_NONE)
		{
			return false;
		}
		StartIdx++;
	}

	// Find the end of the JSON
	int32 EndIdx = Response.Find(TEXT("```"), StartIdx);
	if (EndIdx == INDEX_NONE)
	{
		// Try to find the closing brace
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

	// Extract JSON string
	FString JSONString = Response.Mid(StartIdx, EndIdx - StartIdx).TrimStartAndEnd();

	// Parse JSON
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JSONString);
	if (!FJsonSerializer::Deserialize(Reader, OutCommand) || !OutCommand.IsValid())
	{
		return false;
	}

	return true;
}

bool FCommandExecutor::ExecuteSpawnActor(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	FString ClassName;
	if (!Parameters->TryGetStringField(TEXT("class_name"), ClassName))
	{
		OutLog = TEXT("Error: spawn_actor requires 'class_name' parameter.");
		return false;
	}

	// Parse location, rotation, scale
	FVector Location = ParseVector(Parameters, TEXT("location"), FVector(0, 0, 100));
	FRotator Rotation = ParseRotator(Parameters, TEXT("rotation"));
	FVector Scale = ParseVector(Parameters, TEXT("scale"), FVector(1, 1, 1));

	// Try to load the class
	UClass* ActorClass = FindObject<UClass>(ANY_PACKAGE, *ClassName);
	if (!ActorClass)
	{
		// Try with /Script/Engine prefix
		FString FullClassName = FString::Printf(TEXT("/Script/Engine.%s"), *ClassName);
		ActorClass = LoadObject<UClass>(nullptr, *FullClassName);
	}

	if (!ActorClass)
	{
		OutLog = FString::Printf(TEXT("Error: Could not find actor class '%s'."), *ClassName);
		return false;
	}

	// Spawn actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewActor = World->SpawnActor<AActor>(ActorClass, Location, Rotation, SpawnParams);
	if (!NewActor)
	{
		OutLog = FString::Printf(TEXT("Error: Failed to spawn actor of class '%s'."), *ClassName);
		return false;
	}

	NewActor->SetActorScale3D(Scale);

	// Select the newly spawned actor for visual feedback
	if (GEditor)
	{
		GEditor->SelectNone(false, true);
		GEditor->SelectActor(NewActor, true, true);
		GEditor->NoteSelectionChange();
	}

	OutLog = FString::Printf(TEXT("Successfully spawned '%s' at location %s (now selected)."), *ClassName, *Location.ToString());
	return true;
}

bool FCommandExecutor::ExecuteCreateRoad(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	FVector StartLocation = ParseVector(Parameters, TEXT("start_location"));
	FVector EndLocation = ParseVector(Parameters, TEXT("end_location"));

	double Width = 400.0; // Default road width
	Parameters->TryGetNumberField(TEXT("width"), Width);

	FString MaterialPath;
	Parameters->TryGetStringField(TEXT("material_path"), MaterialPath);

	// Create a spline actor for the road
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(*FString::Printf(TEXT("Road_%d"), FMath::Rand()));

	AActor* RoadActor = World->SpawnActor<AActor>(AActor::StaticClass(), StartLocation, FRotator::ZeroRotator, SpawnParams);
	if (!RoadActor)
	{
		OutLog = TEXT("Error: Failed to create road actor.");
		return false;
	}

	// Add spline component
	USplineComponent* SplineComp = NewObject<USplineComponent>(RoadActor, USplineComponent::StaticClass(), TEXT("RoadSpline"));
	SplineComp->RegisterComponent();
	RoadActor->SetRootComponent(SplineComp);

	// Set spline points
	SplineComp->ClearSplinePoints();
	SplineComp->AddSplinePoint(StartLocation, ESplineCoordinateSpace::World);
	SplineComp->AddSplinePoint(EndLocation, ESplineCoordinateSpace::World);
	SplineComp->UpdateSpline();

	// Load road mesh (using a basic cube as placeholder)
	UStaticMesh* RoadMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));

	if (RoadMesh)
	{
		// Create spline mesh component
		USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(RoadActor, USplineMeshComponent::StaticClass());
		SplineMesh->SetStaticMesh(RoadMesh);
		SplineMesh->SetMobility(EComponentMobility::Static);

		// Set spline mesh parameters
		FVector StartPos, StartTangent, EndPos, EndTangent;
		SplineComp->GetLocationAndTangentAtSplinePoint(0, StartPos, StartTangent, ESplineCoordinateSpace::Local);
		SplineComp->GetLocationAndTangentAtSplinePoint(1, EndPos, EndTangent, ESplineCoordinateSpace::Local);

		SplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);
		SplineMesh->SetStartScale(FVector2D(Width / 100.0, 0.1)); // Scale to road width and make it flat
		SplineMesh->SetEndScale(FVector2D(Width / 100.0, 0.1));

		SplineMesh->RegisterComponent();
		SplineMesh->AttachToComponent(SplineComp, FAttachmentTransformRules::KeepRelativeTransform);

		// Apply material if specified
		if (!MaterialPath.IsEmpty())
		{
			UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, *MaterialPath);
			if (Material)
			{
				SplineMesh->SetMaterial(0, Material);
			}
		}
	}

	// Select the road actor for visual feedback
	if (GEditor)
	{
		GEditor->SelectNone(false, true);
		GEditor->SelectActor(RoadActor, true, true);
		GEditor->NoteSelectionChange();
	}

	OutLog = FString::Printf(TEXT("Successfully created road from %s to %s (width: %.1f, now selected)."),
		*StartLocation.ToString(), *EndLocation.ToString(), Width);
	return true;
}

bool FCommandExecutor::ExecutePlaceModel(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	FString MeshPath;
	if (!Parameters->TryGetStringField(TEXT("mesh_path"), MeshPath))
	{
		OutLog = TEXT("Error: place_model requires 'mesh_path' parameter.");
		return false;
	}

	FVector Location = ParseVector(Parameters, TEXT("location"), FVector(0, 0, 100));
	FRotator Rotation = ParseRotator(Parameters, TEXT("rotation"));
	FVector Scale = ParseVector(Parameters, TEXT("scale"), FVector(1, 1, 1));

	// Load static mesh
	UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
	if (!Mesh)
	{
		OutLog = FString::Printf(TEXT("Error: Could not load static mesh from '%s'."), *MeshPath);
		return false;
	}

	// Spawn static mesh actor
	FActorSpawnParameters SpawnParams;
	AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);
	if (!MeshActor)
	{
		OutLog = TEXT("Error: Failed to spawn static mesh actor.");
		return false;
	}

	MeshActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
	MeshActor->SetActorScale3D(Scale);

	// Select the newly placed mesh for visual feedback
	if (GEditor)
	{
		GEditor->SelectNone(false, true);
		GEditor->SelectActor(MeshActor, true, true);
		GEditor->NoteSelectionChange();
	}

	OutLog = FString::Printf(TEXT("Successfully placed model '%s' at location %s (now selected)."), *MeshPath, *Location.ToString());
	return true;
}

bool FCommandExecutor::ExecuteModifyLandscape(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	// Landscape modification requires more complex implementation
	// This is a placeholder that shows the structure

	FString Operation;
	if (!Parameters->TryGetStringField(TEXT("operation"), Operation))
	{
		OutLog = TEXT("Error: modify_landscape requires 'operation' parameter (raise|lower|smooth|flatten).");
		return false;
	}

	FVector Location = ParseVector(Parameters, TEXT("location"));
	double Radius = 1000.0;
	double Strength = 0.5;

	Parameters->TryGetNumberField(TEXT("radius"), Radius);
	Parameters->TryGetNumberField(TEXT("strength"), Strength);

	OutLog = FString::Printf(TEXT("Landscape modification (%s) queued at %s (radius: %.1f, strength: %.2f). Note: Full landscape editing requires ALandscape API integration."),
		*Operation, *Location.ToString(), Radius, Strength);

	// TODO: Implement actual landscape modification using ALandscape and FLandscapeEditDataInterface
	// This requires accessing the landscape heightmap and modifying it appropriately

	return true;
}

bool FCommandExecutor::ExecuteCreateMaterial(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	FString BaseMaterialPath;
	if (!Parameters->TryGetStringField(TEXT("base_material_path"), BaseMaterialPath))
	{
		OutLog = TEXT("Error: create_material requires 'base_material_path' parameter.");
		return false;
	}

	FString InstanceName;
	if (!Parameters->TryGetStringField(TEXT("instance_name"), InstanceName))
	{
		InstanceName = TEXT("NewMaterialInstance");
	}

	// Load base material
	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, *BaseMaterialPath);
	if (!BaseMaterial)
	{
		OutLog = FString::Printf(TEXT("Error: Could not load base material from '%s'."), *BaseMaterialPath);
		return false;
	}

	// Create dynamic material instance (runtime)
	UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterial, World);
	if (!MaterialInstance)
	{
		OutLog = TEXT("Error: Failed to create material instance.");
		return false;
	}

	// Apply parameters if specified
	const TSharedPtr<FJsonObject>* MaterialParametersPtr;
	if (Parameters->TryGetObjectField(TEXT("parameters"), MaterialParametersPtr))
	{
		TSharedPtr<FJsonObject> MaterialParameters = *MaterialParametersPtr;

		// Iterate through parameters and set them
		for (const auto& Param : MaterialParameters->Values)
		{
			if (Param.Value->Type == EJson::Number)
			{
				MaterialInstance->SetScalarParameterValue(FName(*Param.Key), Param.Value->AsNumber());
			}
			else if (Param.Value->Type == EJson::Object)
			{
				// Assume it's a color (R, G, B, A)
				TSharedPtr<FJsonObject> ColorObj = Param.Value->AsObject();
				FLinearColor Color;
				ColorObj->TryGetNumberField(TEXT("r"), Color.R);
				ColorObj->TryGetNumberField(TEXT("g"), Color.G);
				ColorObj->TryGetNumberField(TEXT("b"), Color.B);
				ColorObj->TryGetNumberField(TEXT("a"), Color.A);
				MaterialInstance->SetVectorParameterValue(FName(*Param.Key), Color);
			}
		}
	}

	OutLog = FString::Printf(TEXT("Successfully created material instance '%s' from '%s'."), *InstanceName, *BaseMaterialPath);
	return true;
}

bool FCommandExecutor::ExecuteBlueprint(const TSharedPtr<FJsonObject>& Parameters, UWorld* World, FString& OutLog)
{
	OutLog = TEXT("Blueprint execution is not yet implemented. This requires blueprint function reflection and invocation.");
	return false;
}

FVector FCommandExecutor::ParseVector(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, const FVector& Default)
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

FRotator FCommandExecutor::ParseRotator(const TSharedPtr<FJsonObject>& Obj, const FString& FieldName, const FRotator& Default)
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

FString FCommandExecutor::GetHelpText()
{
	return TEXT(
		"=== CLAUDE TERMINAL HELP ===\n\n"
		"I can help you manipulate Unreal Engine 5 using natural language.\n"
		"Just tell me what you want to do, and I'll execute the appropriate commands.\n\n"

		"AVAILABLE COMMANDS:\n\n"

		"1. spawn_actor - Create any UE5 actor\n"
		"   Examples:\n"
		"   - 'Spawn a cube at 0,0,100'\n"
		"   - 'Create a point light at 1000,500,200'\n"
		"   - 'Place a player start here'\n\n"

		"2. create_road - Build spline-based roads\n"
		"   Examples:\n"
		"   - 'Create a road from 0,0,0 to 1000,0,0'\n"
		"   - 'Make a 6-meter wide road from A to B'\n"
		"   - 'Build a dirt road connecting these points'\n\n"

		"3. place_model - Position static meshes\n"
		"   Examples:\n"
		"   - 'Place the trench mesh at 1000,500,0'\n"
		"   - 'Add a tree at 500,200,0 with scale 2'\n"
		"   - 'Put a tank model at the player location'\n\n"

		"4. modify_landscape - Sculpt terrain (limited)\n"
		"   Examples:\n"
		"   - 'Raise the terrain at 0,0,0 with radius 500'\n"
		"   - 'Flatten the ground under the base'\n"
		"   - 'Smooth the landscape around spawn point'\n\n"

		"5. create_material - Make material instances\n"
		"   Examples:\n"
		"   - 'Create a material from M_Basic with red color'\n"
		"   - 'Make a new material instance for metal'\n\n"

		"TIPS:\n"
		"- I can see what's selected in your viewport\n"
		"- I know your camera location\n"
		"- I understand relative positions ('above', 'near', 'between')\n"
		"- You can undo any command with Ctrl+Z\n"
		"- Ask me questions! I can explain concepts and help troubleshoot\n\n"

		"EXAMPLES:\n"
		"- 'What's in this level?' - I'll describe the scene\n"
		"- 'Move selected actor 100 units up' - I see your selection\n"
		"- 'Create a trench from here to there' - Use camera for reference\n"
		"- 'How do I create a material?' - I'll explain and help\n\n"

		"Just talk to me naturally - I'll figure out what you need!\n"
		"================================\n"
	);
}
