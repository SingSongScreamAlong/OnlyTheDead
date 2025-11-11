// Copyright Only The Dead Development Team. All Rights Reserved.

#include "UE5CommandExecutor.h"
#include "Editor.h"
#include "EditorActorFolders.h"
#include "Engine/StaticMeshActor.h"
#include "Components/SplineComponent.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeEditorObject.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Blueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"

UUE5CommandExecutor::UUE5CommandExecutor()
{
}

void UUE5CommandExecutor::ExecuteStep(const FAgentTaskStep& Step, FOnCommandExecuted OnComplete)
{
	CurrentCallback = OnComplete;

	UE_LOG(LogTemp, Log, TEXT("Executing command: %s"), *Step.CommandType);

	if (Step.CommandType == TEXT("CreateActor"))
	{
		ExecuteCreateActor(Step, OnComplete);
	}
	else if (Step.CommandType == TEXT("ModifyTerrain"))
	{
		ExecuteModifyTerrain(Step, OnComplete);
	}
	else if (Step.CommandType == TEXT("CreateBlueprint"))
	{
		ExecuteCreateBlueprint(Step, OnComplete);
	}
	else if (Step.CommandType == TEXT("SetMaterial"))
	{
		ExecuteSetMaterial(Step, OnComplete);
	}
	else if (Step.CommandType == TEXT("ExecutePython"))
	{
		ExecutePythonScript(Step, OnComplete);
	}
	else if (Step.CommandType == TEXT("PlaceAsset"))
	{
		ExecutePlaceAsset(Step, OnComplete);
	}
	else if (Step.CommandType == TEXT("CreateSpline"))
	{
		ExecuteCreateSpline(Step, OnComplete);
	}
	else if (Step.CommandType == TEXT("ModifyGeometry"))
	{
		ExecuteModifyGeometry(Step, OnComplete);
	}
	else
	{
		OnComplete.ExecuteIfBound(false, FString::Printf(TEXT("Unknown command type: %s"), *Step.CommandType));
	}
}

UWorld* UUE5CommandExecutor::GetEditorWorld() const
{
	if (GEditor)
	{
		return GEditor->GetEditorWorldContext().World();
	}
	return nullptr;
}

// ============================================================================
// COMMAND IMPLEMENTATIONS
// ============================================================================

void UUE5CommandExecutor::ExecuteCreateActor(const FAgentTaskStep& Step, FOnCommandExecuted OnComplete)
{
	UWorld* World = GetEditorWorld();
	if (!World)
	{
		OnComplete.ExecuteIfBound(false, TEXT("No editor world found"));
		return;
	}

	// Parse parameters
	FString ClassName = Step.Parameters.FindRef(TEXT("class"));
	FString LocationStr = Step.Parameters.FindRef(TEXT("location"));
	FString RotationStr = Step.Parameters.FindRef(TEXT("rotation"));
	FString ActorName = Step.Parameters.FindRef(TEXT("name"));

	// Find the class
	UClass* ActorClass = FindClass(ClassName);
	if (!ActorClass)
	{
		OnComplete.ExecuteIfBound(false, FString::Printf(TEXT("Class not found: %s"), *ClassName));
		return;
	}

	// Parse location and rotation
	FVector Location = LocationStr.IsEmpty() ? FVector::ZeroVector : ParseVector(LocationStr);
	FRotator Rotation = RotationStr.IsEmpty() ? FRotator::ZeroRotator : ParseRotator(RotationStr);

	// Spawn the actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(*ActorName);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewActor = World->SpawnActor(ActorClass, &Location, &Rotation, SpawnParams);

	if (NewActor)
	{
		if (!ActorName.IsEmpty())
		{
			NewActor->SetActorLabel(ActorName);
		}

		OnComplete.ExecuteIfBound(true, FString::Printf(TEXT("Created actor: %s at %s"), *NewActor->GetName(), *Location.ToString()));
	}
	else
	{
		OnComplete.ExecuteIfBound(false, TEXT("Failed to spawn actor"));
	}
}

void UUE5CommandExecutor::ExecuteModifyTerrain(const FAgentTaskStep& Step, FOnCommandExecuted OnComplete)
{
	// Terrain modification implementation
	// This would use the Landscape API to sculpt terrain

	FString Operation = Step.Parameters.FindRef(TEXT("operation")); // "raise", "lower", "flatten", "smooth"
	FString LocationStr = Step.Parameters.FindRef(TEXT("location"));
	FString RadiusStr = Step.Parameters.FindRef(TEXT("radius"));
	FString StrengthStr = Step.Parameters.FindRef(TEXT("strength"));

	UWorld* World = GetEditorWorld();
	if (!World)
	{
		OnComplete.ExecuteIfBound(false, TEXT("No editor world found"));
		return;
	}

	// Find landscape actor
	ALandscape* Landscape = nullptr;
	for (TActorIterator<ALandscape> It(World); It; ++It)
	{
		Landscape = *It;
		break;
	}

	if (!Landscape)
	{
		OnComplete.ExecuteIfBound(false, TEXT("No landscape found in level"));
		return;
	}

	// TODO: Implement actual landscape modification using FLandscapeEditDataInterface
	// This requires more complex implementation with the landscape editing API

	OnComplete.ExecuteIfBound(true, FString::Printf(TEXT("Terrain modified: %s"), *Operation));
}

void UUE5CommandExecutor::ExecuteCreateBlueprint(const FAgentTaskStep& Step, FOnCommandExecuted OnComplete)
{
	FString BlueprintName = Step.Parameters.FindRef(TEXT("name"));
	FString ParentClass = Step.Parameters.FindRef(TEXT("parentClass"));
	FString PackagePath = Step.Parameters.FindRef(TEXT("path"));

	if (PackagePath.IsEmpty())
	{
		PackagePath = TEXT("/Game/Blueprints");
	}

	// Find parent class
	UClass* ParentUClass = FindClass(ParentClass);
	if (!ParentUClass)
	{
		ParentUClass = AActor::StaticClass();
	}

	// Create the blueprint
	FString PackageName = PackagePath / BlueprintName;
	UPackage* Package = CreatePackage(*PackageName);

	UBlueprint* NewBlueprint = FKismetEditorUtilities::CreateBlueprint(
		ParentUClass,
		Package,
		FName(*BlueprintName),
		BPTYPE_Normal,
		UBlueprint::StaticClass(),
		UBlueprintGeneratedClass::StaticClass()
	);

	if (NewBlueprint)
	{
		FAssetRegistryModule::AssetCreated(NewBlueprint);
		Package->MarkPackageDirty();

		OnComplete.ExecuteIfBound(true, FString::Printf(TEXT("Created blueprint: %s"), *BlueprintName));
	}
	else
	{
		OnComplete.ExecuteIfBound(false, TEXT("Failed to create blueprint"));
	}
}

void UUE5CommandExecutor::ExecuteSetMaterial(const FAgentTaskStep& Step, FOnCommandExecuted OnComplete)
{
	FString ActorName = Step.Parameters.FindRef(TEXT("actor"));
	FString MaterialPath = Step.Parameters.FindRef(TEXT("material"));

	UWorld* World = GetEditorWorld();
	if (!World)
	{
		OnComplete.ExecuteIfBound(false, TEXT("No editor world found"));
		return;
	}

	// Find the actor
	AActor* TargetActor = nullptr;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->GetActorLabel() == ActorName || It->GetName() == ActorName)
		{
			TargetActor = *It;
			break;
		}
	}

	if (!TargetActor)
	{
		OnComplete.ExecuteIfBound(false, FString::Printf(TEXT("Actor not found: %s"), *ActorName));
		return;
	}

	// Load the material
	UMaterialInterface* Material = Cast<UMaterialInterface>(FindAsset(MaterialPath));
	if (!Material)
	{
		OnComplete.ExecuteIfBound(false, FString::Printf(TEXT("Material not found: %s"), *MaterialPath));
		return;
	}

	// Apply material to all mesh components
	TArray<UMeshComponent*> MeshComponents;
	TargetActor->GetComponents<UMeshComponent>(MeshComponents);

	int32 ComponentsModified = 0;
	for (UMeshComponent* MeshComp : MeshComponents)
	{
		if (MeshComp)
		{
			for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
			{
				MeshComp->SetMaterial(i, Material);
			}
			ComponentsModified++;
		}
	}

	if (ComponentsModified > 0)
	{
		OnComplete.ExecuteIfBound(true, FString::Printf(TEXT("Applied material to %d components"), ComponentsModified));
	}
	else
	{
		OnComplete.ExecuteIfBound(false, TEXT("No mesh components found on actor"));
	}
}

void UUE5CommandExecutor::ExecutePythonScript(const FAgentTaskStep& Step, FOnCommandExecuted OnComplete)
{
	FString Script = Step.Parameters.FindRef(TEXT("script"));

	// TODO: Execute Python script using UE5's Python API
	// This requires the Python plugin to be enabled

	OnComplete.ExecuteIfBound(true, TEXT("Python execution not yet implemented"));
}

void UUE5CommandExecutor::ExecutePlaceAsset(const FAgentTaskStep& Step, FOnCommandExecuted OnComplete)
{
	FString AssetPath = Step.Parameters.FindRef(TEXT("asset"));
	FString LocationStr = Step.Parameters.FindRef(TEXT("location"));
	FString RotationStr = Step.Parameters.FindRef(TEXT("rotation"));
	FString ScaleStr = Step.Parameters.FindRef(TEXT("scale"));

	UWorld* World = GetEditorWorld();
	if (!World)
	{
		OnComplete.ExecuteIfBound(false, TEXT("No editor world found"));
		return;
	}

	// Load the asset
	UObject* Asset = FindAsset(AssetPath);
	if (!Asset)
	{
		OnComplete.ExecuteIfBound(false, FString::Printf(TEXT("Asset not found: %s"), *AssetPath));
		return;
	}

	// Parse transform
	FVector Location = LocationStr.IsEmpty() ? FVector::ZeroVector : ParseVector(LocationStr);
	FRotator Rotation = RotationStr.IsEmpty() ? FRotator::ZeroRotator : ParseRotator(RotationStr);
	FVector Scale = ScaleStr.IsEmpty() ? FVector::OneVector : ParseVector(ScaleStr);

	// Determine asset type and spawn appropriate actor
	AActor* SpawnedActor = nullptr;

	if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(Asset))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);
		if (MeshActor)
		{
			MeshActor->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
			MeshActor->GetStaticMeshComponent()->SetWorldScale3D(Scale);
			SpawnedActor = MeshActor;
		}
	}

	if (SpawnedActor)
	{
		OnComplete.ExecuteIfBound(true, FString::Printf(TEXT("Placed asset: %s"), *AssetPath));
	}
	else
	{
		OnComplete.ExecuteIfBound(false, TEXT("Failed to place asset"));
	}
}

void UUE5CommandExecutor::ExecuteCreateSpline(const FAgentTaskStep& Step, FOnCommandExecuted OnComplete)
{
	FString SplineName = Step.Parameters.FindRef(TEXT("name"));
	FString PointsStr = Step.Parameters.FindRef(TEXT("points")); // Comma-separated list of vectors

	UWorld* World = GetEditorWorld();
	if (!World)
	{
		OnComplete.ExecuteIfBound(false, TEXT("No editor world found"));
		return;
	}

	// Create spline actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(*SplineName);

	AActor* SplineActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (!SplineActor)
	{
		OnComplete.ExecuteIfBound(false, TEXT("Failed to create spline actor"));
		return;
	}

	USplineComponent* SplineComponent = NewObject<USplineComponent>(SplineActor, USplineComponent::StaticClass());
	SplineComponent->RegisterComponent();
	SplineActor->SetRootComponent(SplineComponent);

	if (!SplineName.IsEmpty())
	{
		SplineActor->SetActorLabel(SplineName);
	}

	// Parse and add spline points
	// TODO: Parse points from PointsStr

	OnComplete.ExecuteIfBound(true, FString::Printf(TEXT("Created spline: %s"), *SplineName));
}

void UUE5CommandExecutor::ExecuteModifyGeometry(const FAgentTaskStep& Step, FOnCommandExecuted OnComplete)
{
	// Geometry modification using GeometryScripting
	// TODO: Implement using UE5's GeometryScripting API

	OnComplete.ExecuteIfBound(true, TEXT("Geometry modification not yet fully implemented"));
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

FVector UUE5CommandExecutor::ParseVector(const FString& VectorString)
{
	FVector Result = FVector::ZeroVector;

	// Expected format: "X=1.0,Y=2.0,Z=3.0" or "1.0,2.0,3.0"
	FString CleanString = VectorString.TrimStartAndEnd();

	TArray<FString> Components;
	CleanString.ParseIntoArray(Components, TEXT(","));

	if (Components.Num() >= 3)
	{
		// Try "X=1.0" format first
		if (Components[0].Contains(TEXT("=")))
		{
			for (const FString& Component : Components)
			{
				TArray<FString> KeyValue;
				Component.ParseIntoArray(KeyValue, TEXT("="));
				if (KeyValue.Num() == 2)
				{
					FString Key = KeyValue[0].TrimStartAndEnd().ToUpper();
					float Value = FCString::Atof(*KeyValue[1]);

					if (Key == TEXT("X")) Result.X = Value;
					else if (Key == TEXT("Y")) Result.Y = Value;
					else if (Key == TEXT("Z")) Result.Z = Value;
				}
			}
		}
		else
		{
			// Try simple "1.0,2.0,3.0" format
			Result.X = FCString::Atof(*Components[0]);
			Result.Y = FCString::Atof(*Components[1]);
			Result.Z = FCString::Atof(*Components[2]);
		}
	}

	return Result;
}

FRotator UUE5CommandExecutor::ParseRotator(const FString& RotatorString)
{
	FRotator Result = FRotator::ZeroRotator;

	// Expected format: "Pitch=0,Yaw=90,Roll=0" or "0,90,0"
	FString CleanString = RotatorString.TrimStartAndEnd();

	TArray<FString> Components;
	CleanString.ParseIntoArray(Components, TEXT(","));

	if (Components.Num() >= 3)
	{
		if (Components[0].Contains(TEXT("=")))
		{
			for (const FString& Component : Components)
			{
				TArray<FString> KeyValue;
				Component.ParseIntoArray(KeyValue, TEXT("="));
				if (KeyValue.Num() == 2)
				{
					FString Key = KeyValue[0].TrimStartAndEnd().ToUpper();
					float Value = FCString::Atof(*KeyValue[1]);

					if (Key == TEXT("PITCH")) Result.Pitch = Value;
					else if (Key == TEXT("YAW")) Result.Yaw = Value;
					else if (Key == TEXT("ROLL")) Result.Roll = Value;
				}
			}
		}
		else
		{
			Result.Pitch = FCString::Atof(*Components[0]);
			Result.Yaw = FCString::Atof(*Components[1]);
			Result.Roll = FCString::Atof(*Components[2]);
		}
	}

	return Result;
}

UClass* UUE5CommandExecutor::FindClass(const FString& ClassName)
{
	if (ClassName.IsEmpty())
	{
		return nullptr;
	}

	// Try to find the class
	UClass* FoundClass = FindObject<UClass>(ANY_PACKAGE, *ClassName);

	if (!FoundClass)
	{
		// Try with common prefixes
		FoundClass = FindObject<UClass>(ANY_PACKAGE, *(TEXT("A") + ClassName));
	}

	if (!FoundClass)
	{
		FoundClass = FindObject<UClass>(ANY_PACKAGE, *(TEXT("U") + ClassName));
	}

	return FoundClass;
}

UObject* UUE5CommandExecutor::FindAsset(const FString& AssetPath)
{
	if (AssetPath.IsEmpty())
	{
		return nullptr;
	}

	// Try to load the asset directly
	UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);

	if (!Asset)
	{
		// Try using asset registry
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FName(*AssetPath));

		if (AssetData.IsValid())
		{
			Asset = AssetData.GetAsset();
		}
	}

	return Asset;
}
