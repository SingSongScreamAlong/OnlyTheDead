// Copyright Epic Games, Inc. All Rights Reserved.

#include "SceneContextBuilder.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Light.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Selection.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "LevelEditorViewport.h"

FSceneContextBuilder::FSceneContextBuilder()
{
}

FSceneContextBuilder::~FSceneContextBuilder()
{
}

FString FSceneContextBuilder::BuildContext(UWorld* World, bool bIncludeSelection, int32 MaxActors)
{
	if (!World)
	{
		return TEXT("");
	}

	FString Context;
	Context.Append(TEXT("\n=== SCENE CONTEXT ===\n\n"));

	// Level info
	Context.Append(GetLevelInfo(World));
	Context.Append(TEXT("\n"));

	// Camera info
	FString CameraInfo = GetCameraInfo();
	if (!CameraInfo.IsEmpty())
	{
		Context.Append(CameraInfo);
		Context.Append(TEXT("\n"));
	}

	// Selected actors
	if (bIncludeSelection)
	{
		FString SelectionInfo = BuildSelectionContext();
		if (!SelectionInfo.IsEmpty())
		{
			Context.Append(SelectionInfo);
			Context.Append(TEXT("\n"));
		}
	}

	// World actors (sample)
	Context.Append(TEXT("Actors in Level:\n"));

	TArray<AActor*> Actors;
	int32 ActorCount = 0;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || Actor->IsHiddenEd() || Actor->IsA(AWorldSettings::StaticClass()))
		{
			continue;
		}

		if (ActorCount < MaxActors)
		{
			Context.Append(TEXT("  - "));
			Context.Append(FormatActorInfo(Actor, false));
			Context.Append(TEXT("\n"));
		}

		ActorCount++;
	}

	if (ActorCount > MaxActors)
	{
		Context.Append(FString::Printf(TEXT("  ... and %d more actors\n"), ActorCount - MaxActors));
	}

	Context.Append(FString::Printf(TEXT("\nTotal: %d actors in scene\n"), ActorCount));
	Context.Append(TEXT("===================\n\n"));

	return Context;
}

FString FSceneContextBuilder::BuildSelectionContext()
{
	if (!GEditor)
	{
		return TEXT("");
	}

	USelection* Selection = GEditor->GetSelectedActors();
	if (!Selection || Selection->Num() == 0)
	{
		return TEXT("");
	}

	FString Context;
	Context.Append(FString::Printf(TEXT("Selected Actors (%d):\n"), Selection->Num()));

	for (int32 i = 0; i < Selection->Num(); ++i)
	{
		AActor* Actor = Cast<AActor>(Selection->GetSelectedObject(i));
		if (Actor)
		{
			Context.Append(TEXT("  - "));
			Context.Append(FormatActorInfo(Actor, true));
			Context.Append(TEXT("\n"));
		}
	}

	return Context;
}

FString FSceneContextBuilder::GetCameraInfo()
{
	if (!GEditor)
	{
		return TEXT("");
	}

	FLevelEditorViewportClient* ViewportClient = nullptr;

	// Try to get the active viewport
	for (FLevelEditorViewportClient* Client : GEditor->GetLevelViewportClients())
	{
		if (Client && Client->IsPerspective())
		{
			ViewportClient = Client;
			break;
		}
	}

	if (!ViewportClient)
	{
		return TEXT("");
	}

	FVector CameraLocation = ViewportClient->GetViewLocation();
	FRotator CameraRotation = ViewportClient->GetViewRotation();

	FString Context;
	Context.Append(TEXT("Editor Camera:\n"));
	Context.Append(FString::Printf(TEXT("  Location: %s\n"), *FormatVector(CameraLocation)));
	Context.Append(FString::Printf(TEXT("  Rotation: %s\n"), *FormatRotator(CameraRotation)));

	return Context;
}

FString FSceneContextBuilder::GetLevelInfo(UWorld* World)
{
	if (!World)
	{
		return TEXT("");
	}

	FString LevelName = World->GetMapName();
	FBox LevelBounds = FBox(ForceInit);

	// Calculate level bounds
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && !Actor->IsHiddenEd())
		{
			LevelBounds += Actor->GetActorLocation();
		}
	}

	FString Context;
	Context.Append(FString::Printf(TEXT("Level: %s\n"), *LevelName));

	if (LevelBounds.IsValid)
	{
		FVector Center = LevelBounds.GetCenter();
		FVector Extent = LevelBounds.GetExtent();
		Context.Append(FString::Printf(TEXT("Bounds Center: %s\n"), *FormatVector(Center)));
		Context.Append(FString::Printf(TEXT("Bounds Extent: %s\n"), *FormatVector(Extent)));
	}

	return Context;
}

FString FSceneContextBuilder::FormatActorInfo(AActor* Actor, bool bDetailed)
{
	if (!Actor)
	{
		return TEXT("Invalid Actor");
	}

	FString Info;

	// Actor name and type
	Info.Append(FString::Printf(TEXT("%s (%s)"), *Actor->GetActorLabel(), *GetActorTypeName(Actor)));

	if (bDetailed)
	{
		// Location
		FVector Location = Actor->GetActorLocation();
		Info.Append(FString::Printf(TEXT(" at %s"), *FormatVector(Location)));

		// Rotation if not zero
		FRotator Rotation = Actor->GetActorRotation();
		if (!Rotation.IsZero())
		{
			Info.Append(FString::Printf(TEXT(", rotation %s"), *FormatRotator(Rotation)));
		}

		// Scale if not (1,1,1)
		FVector Scale = Actor->GetActorScale3D();
		if (!Scale.Equals(FVector::OneVector))
		{
			Info.Append(FString::Printf(TEXT(", scale %s"), *FormatVector(Scale)));
		}

		// Additional type-specific info
		if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
		{
			if (UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
			{
				if (UStaticMesh* Mesh = MeshComp->GetStaticMesh())
				{
					Info.Append(FString::Printf(TEXT(", mesh: %s"), *Mesh->GetName()));
				}
			}
		}
	}
	else
	{
		// Just location in compact form
		FVector Location = Actor->GetActorLocation();
		Info.Append(FString::Printf(TEXT(" at %s"), *FormatVector(Location)));
	}

	return Info;
}

FString FSceneContextBuilder::GetActorTypeName(AActor* Actor)
{
	if (!Actor)
	{
		return TEXT("Unknown");
	}

	// Return friendly names for common types
	if (Actor->IsA(AStaticMeshActor::StaticClass()))
	{
		return TEXT("StaticMesh");
	}
	if (Actor->IsA(ALight::StaticClass()))
	{
		return TEXT("Light");
	}
	if (Actor->IsA(APlayerStart::StaticClass()))
	{
		return TEXT("PlayerStart");
	}

	// Default to class name
	return Actor->GetClass()->GetName();
}

FString FSceneContextBuilder::FormatVector(const FVector& Vec)
{
	return FString::Printf(TEXT("(%.1f, %.1f, %.1f)"), Vec.X, Vec.Y, Vec.Z);
}

FString FSceneContextBuilder::FormatRotator(const FRotator& Rot)
{
	return FString::Printf(TEXT("(P=%.1f, Y=%.1f, R=%.1f)"), Rot.Pitch, Rot.Yaw, Rot.Roll);
}
