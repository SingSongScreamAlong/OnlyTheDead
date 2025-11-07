// Copyright Epic Games, Inc. All Rights Reserved.

#include "TemplateSystem.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "HAL/PlatformFileManager.h"

bool FTemplateSystem::LoadTemplate(const FString& TemplatePath, FSceneTemplate& OutTemplate)
{
	FString JSONString;
	if (!FFileHelper::LoadFileToString(JSONString, *TemplatePath))
	{
		UE_LOG(LogTemp, Error, TEXT("TemplateSystem: Failed to load template file: %s"), *TemplatePath);
		return false;
	}

	return ParseTemplateJSON(JSONString, OutTemplate);
}

bool FTemplateSystem::SaveTemplate(const FSceneTemplate& Template, const FString& TemplatePath)
{
	FString ErrorMsg;
	if (!ValidateTemplate(Template, ErrorMsg))
	{
		UE_LOG(LogTemp, Error, TEXT("TemplateSystem: Template validation failed: %s"), *ErrorMsg);
		return false;
	}

	FString JSONString = GenerateTemplateJSON(Template);

	// Ensure directory exists
	FString Directory = FPaths::GetPath(TemplatePath);
	if (!FPaths::DirectoryExists(Directory))
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (!PlatformFile.CreateDirectoryTree(*Directory))
		{
			UE_LOG(LogTemp, Error, TEXT("TemplateSystem: Failed to create directory: %s"), *Directory);
			return false;
		}
	}

	if (!FFileHelper::SaveStringToFile(JSONString, *TemplatePath))
	{
		UE_LOG(LogTemp, Error, TEXT("TemplateSystem: Failed to save template file: %s"), *TemplatePath);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("TemplateSystem: Saved template '%s' to %s"), *Template.TemplateName, *TemplatePath);
	return true;
}

bool FTemplateSystem::SpawnTemplate(
	const FSceneTemplate& Template,
	const FVector& SpawnLocation,
	const FRotator& SpawnRotation,
	UWorld* World,
	FTemplateSpawnResult& OutResult)
{
	if (!World)
	{
		OutResult.ErrorMessage = TEXT("World is null");
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("TemplateSystem: Spawning template '%s' with %d elements"),
		*Template.TemplateName, Template.Elements.Num());

	for (const FTemplateElement& Element : Template.Elements)
	{
		AActor* SpawnedActor = SpawnTemplateElement(Element, SpawnLocation, SpawnRotation, World);
		if (SpawnedActor)
		{
			OutResult.SpawnedActors.Add(SpawnedActor);
			OutResult.TotalElementsSpawned++;
		}
	}

	OutResult.bSuccess = OutResult.TotalElementsSpawned > 0;

	if (OutResult.bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("TemplateSystem: Successfully spawned %d/%d elements"),
			OutResult.TotalElementsSpawned, Template.Elements.Num());
	}
	else
	{
		OutResult.ErrorMessage = TEXT("No elements were spawned");
	}

	return OutResult.bSuccess;
}

bool FTemplateSystem::CreateTemplateFromSelection(
	const TArray<AActor*>& SelectedActors,
	const FString& TemplateName,
	FSceneTemplate& OutTemplate)
{
	if (SelectedActors.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("TemplateSystem: No actors selected"));
		return false;
	}

	OutTemplate.TemplateName = TemplateName;
	OutTemplate.Description = FString::Printf(TEXT("Template created from %d selected actors"), SelectedActors.Num());
	OutTemplate.CreatedDate = FDateTime::Now();

	// Calculate center point
	FVector Center = FVector::ZeroVector;
	for (AActor* Actor : SelectedActors)
	{
		if (Actor)
		{
			Center += Actor->GetActorLocation();
		}
	}
	Center /= SelectedActors.Num();

	// Create template elements
	for (AActor* Actor : SelectedActors)
	{
		if (!Actor)
			continue;

		FTemplateElement Element;
		Element.ElementType = Actor->GetClass()->GetName();
		Element.RelativeLocation = Actor->GetActorLocation() - Center;
		Element.RelativeRotation = Actor->GetActorRotation();
		Element.Scale = Actor->GetActorScale3D();

		// Try to get static mesh actor asset path
		if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
		{
			if (UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
			{
				if (UStaticMesh* Mesh = MeshComp->GetStaticMesh())
				{
					Element.AssetPath = Mesh->GetPathName();
				}
			}
		}

		OutTemplate.Elements.Add(Element);
	}

	UE_LOG(LogTemp, Log, TEXT("TemplateSystem: Created template '%s' with %d elements"),
		*TemplateName, OutTemplate.Elements.Num());

	return true;
}

int32 FTemplateSystem::SearchTemplates(const TArray<FString>& SearchTags, TArray<FSceneTemplate>& OutTemplates)
{
	OutTemplates.Empty();

	// Get all templates
	TArray<FSceneTemplate> AllTemplates;
	GetAllTemplates(AllTemplates);

	// Filter by tags
	for (const FSceneTemplate& Template : AllTemplates)
	{
		bool bMatches = false;
		for (const FString& SearchTag : SearchTags)
		{
			if (Template.Tags.Contains(SearchTag))
			{
				bMatches = true;
				break;
			}
		}

		if (bMatches)
		{
			OutTemplates.Add(Template);
		}
	}

	return OutTemplates.Num();
}

int32 FTemplateSystem::GetAllTemplates(TArray<FSceneTemplate>& OutTemplates)
{
	OutTemplates.Empty();

	// Add built-in templates
	OutTemplates.Add(GenerateWWIDefensivePosition());
	OutTemplates.Add(GenerateMedievalVillage());
	OutTemplates.Add(GenerateForestClearing());
	OutTemplates.Add(GenerateMilitaryOutpost());

	// Load user templates from disk
	FString TemplateDir = GetTemplateDirectory();
	TArray<FString> TemplateFiles;
	IFileManager::Get().FindFiles(TemplateFiles, *(TemplateDir / TEXT("*.json")), true, false);

	for (const FString& FileName : TemplateFiles)
	{
		FSceneTemplate Template;
		if (LoadTemplate(TemplateDir / FileName, Template))
		{
			OutTemplates.Add(Template);
		}
	}

	return OutTemplates.Num();
}

FSceneTemplate FTemplateSystem::GenerateWWIDefensivePosition()
{
	FSceneTemplate Template;
	Template.TemplateName = TEXT("WWI French Defensive Position");
	Template.Description = TEXT("Complete WWI trench system with defenses");
	Template.Tags = {TEXT("wwi"), TEXT("french"), TEXT("trench"), TEXT("defensive"), TEXT("military")};

	// Trench line
	FTemplateElement Trench;
	Trench.ElementType = TEXT("trench");
	Trench.Parameters.Add(TEXT("length"), TEXT("5000"));
	Trench.Parameters.Add(TEXT("pattern"), TEXT("zigzag"));
	Template.Elements.Add(Trench);

	// Sandbags
	for (int32 i = 0; i < 20; i++)
	{
		FTemplateElement Sandbag;
		Sandbag.ElementType = TEXT("sandbag");
		Sandbag.RelativeLocation = FVector(i * 200.0f, 0, 0);
		Template.Elements.Add(Sandbag);
	}

	// Barbed wire
	FTemplateElement BarbedWire;
	BarbedWire.ElementType = TEXT("barbed_wire");
	BarbedWire.RelativeLocation = FVector(1000, 0, 0);
	Template.Elements.Add(BarbedWire);

	return Template;
}

FSceneTemplate FTemplateSystem::GenerateMedievalVillage()
{
	FSceneTemplate Template;
	Template.TemplateName = TEXT("Medieval Village");
	Template.Description = TEXT("Small medieval village with 15-20 buildings");
	Template.Tags = {TEXT("medieval"), TEXT("village"), TEXT("settlement"), TEXT("historical")};

	// Buildings in organic layout
	for (int32 i = 0; i < 18; i++)
	{
		FTemplateElement Building;
		Building.ElementType = TEXT("medieval_house");

		// Organic placement
		float Angle = (i / 18.0f) * 360.0f;
		float Radius = 500.0f + FMath::RandRange(-200.0f, 200.0f);
		Building.RelativeLocation = FVector(
			FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius,
			FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius,
			0);
		Building.RelativeRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);

		Template.Elements.Add(Building);
	}

	// Central well
	FTemplateElement Well;
	Well.ElementType = TEXT("well");
	Well.RelativeLocation = FVector::ZeroVector;
	Template.Elements.Add(Well);

	return Template;
}

FSceneTemplate FTemplateSystem::GenerateForestClearing()
{
	FSceneTemplate Template;
	Template.TemplateName = TEXT("Forest Clearing");
	Template.Description = TEXT("Natural forest clearing with scattered trees");
	Template.Tags = {TEXT("forest"), TEXT("nature"), TEXT("vegetation"), TEXT("clearing")};

	// Trees around perimeter
	for (int32 i = 0; i < 40; i++)
	{
		FTemplateElement Tree;
		Tree.ElementType = TEXT("tree");

		float Angle = (i / 40.0f) * 360.0f;
		float Radius = 2000.0f + FMath::RandRange(-300.0f, 300.0f);
		Tree.RelativeLocation = FVector(
			FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius,
			FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius,
			0);
		Tree.Scale = FVector(FMath::RandRange(0.8f, 1.5f));

		Template.Elements.Add(Tree);
	}

	// Undergrowth
	for (int32 i = 0; i < 60; i++)
	{
		FTemplateElement Bush;
		Bush.ElementType = TEXT("bush");
		Bush.RelativeLocation = FMath::RandPointInCircle(1500.0f);
		Template.Elements.Add(Bush);
	}

	return Template;
}

FSceneTemplate FTemplateSystem::GenerateMilitaryOutpost()
{
	FSceneTemplate Template;
	Template.TemplateName = TEXT("Military Outpost");
	Template.Description = TEXT("Fortified military outpost with watchtowers and walls");
	Template.Tags = {TEXT("military"), TEXT("outpost"), TEXT("defensive"), TEXT("fortified")};

	// Main building
	FTemplateElement MainBuilding;
	MainBuilding.ElementType = TEXT("military_hq");
	MainBuilding.RelativeLocation = FVector::ZeroVector;
	Template.Elements.Add(MainBuilding);

	// Watchtowers at corners
	TArray<FVector> TowerLocations = {
		FVector(1000, 1000, 0),
		FVector(1000, -1000, 0),
		FVector(-1000, 1000, 0),
		FVector(-1000, -1000, 0)
	};

	for (const FVector& Loc : TowerLocations)
	{
		FTemplateElement Tower;
		Tower.ElementType = TEXT("watchtower");
		Tower.RelativeLocation = Loc;
		Template.Elements.Add(Tower);
	}

	// Defensive walls
	FTemplateElement Wall;
	Wall.ElementType = TEXT("defensive_wall");
	Wall.Parameters.Add(TEXT("length"), TEXT("2000"));
	Template.Elements.Add(Wall);

	return Template;
}

bool FTemplateSystem::ParseTemplateJSON(const FString& JSONString, FSceneTemplate& OutTemplate)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JSONString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("TemplateSystem: Failed to parse JSON"));
		return false;
	}

	// Parse basic fields
	JsonObject->TryGetStringField(TEXT("name"), OutTemplate.TemplateName);
	JsonObject->TryGetStringField(TEXT("description"), OutTemplate.Description);

	// Parse tags
	const TArray<TSharedPtr<FJsonValue>>* TagsArray;
	if (JsonObject->TryGetArrayField(TEXT("tags"), TagsArray))
	{
		for (const TSharedPtr<FJsonValue>& TagValue : *TagsArray)
		{
			OutTemplate.Tags.Add(TagValue->AsString());
		}
	}

	// Parse elements (simplified - full implementation would parse all fields)
	const TArray<TSharedPtr<FJsonValue>>* ElementsArray;
	if (JsonObject->TryGetArrayField(TEXT("elements"), ElementsArray))
	{
		for (const TSharedPtr<FJsonValue>& ElementValue : *ElementsArray)
		{
			const TSharedPtr<FJsonObject>& ElementObj = ElementValue->AsObject();
			if (ElementObj.IsValid())
			{
				FTemplateElement Element;
				ElementObj->TryGetStringField(TEXT("type"), Element.ElementType);
				ElementObj->TryGetStringField(TEXT("asset"), Element.AssetPath);
				OutTemplate.Elements.Add(Element);
			}
		}
	}

	return true;
}

FString FTemplateSystem::GenerateTemplateJSON(const FSceneTemplate& Template)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField(TEXT("name"), Template.TemplateName);
	JsonObject->SetStringField(TEXT("description"), Template.Description);

	// Tags array
	TArray<TSharedPtr<FJsonValue>> TagsArray;
	for (const FString& Tag : Template.Tags)
	{
		TagsArray.Add(MakeShareable(new FJsonValueString(Tag)));
	}
	JsonObject->SetArrayField(TEXT("tags"), TagsArray);

	// Elements array (simplified)
	TArray<TSharedPtr<FJsonValue>> ElementsArray;
	for (const FTemplateElement& Element : Template.Elements)
	{
		TSharedPtr<FJsonObject> ElementObj = MakeShareable(new FJsonObject());
		ElementObj->SetStringField(TEXT("type"), Element.ElementType);
		ElementObj->SetStringField(TEXT("asset"), Element.AssetPath);
		ElementsArray.Add(MakeShareable(new FJsonValueObject(ElementObj)));
	}
	JsonObject->SetArrayField(TEXT("elements"), ElementsArray);

	// Serialize to string
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	return OutputString;
}

AActor* FTemplateSystem::SpawnTemplateElement(
	const FTemplateElement& Element,
	const FVector& BaseLocation,
	const FRotator& BaseRotation,
	UWorld* World)
{
	if (!World)
		return nullptr;

	// Calculate final transform
	FVector FinalLocation = BaseLocation + BaseRotation.RotateVector(Element.RelativeLocation);
	FRotator FinalRotation = BaseRotation + Element.RelativeRotation;

	// Load asset if path provided
	if (!Element.AssetPath.IsEmpty())
	{
		UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *Element.AssetPath);
		if (Mesh)
		{
			FActorSpawnParameters SpawnParams;
			AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(FinalLocation, FinalRotation, SpawnParams);
			if (Actor)
			{
				Actor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
				Actor->SetActorScale3D(Element.Scale);
				return Actor;
			}
		}
	}

	return nullptr;
}

FString FTemplateSystem::GetTemplateDirectory()
{
	return FPaths::ProjectSavedDir() / TEXT("ClaudeTerminal") / TEXT("Templates");
}

bool FTemplateSystem::ValidateTemplate(const FSceneTemplate& Template, FString& OutError)
{
	if (Template.TemplateName.IsEmpty())
	{
		OutError = TEXT("Template name is empty");
		return false;
	}

	if (Template.Elements.Num() == 0)
	{
		OutError = TEXT("Template has no elements");
		return false;
	}

	return true;
}
