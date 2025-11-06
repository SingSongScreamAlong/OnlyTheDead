// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetPreviewSystem.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"

FAssetPreviewSystem::FAssetPreviewSystem()
	: PreviewActor(nullptr)
	, PreviewWorld(nullptr)
	, PreviewScale(FVector::OneVector)
	, PreviewRotation(FRotator::ZeroRotator)
	, GhostMaterial(nullptr)
{
	// Try to load a translucent material for ghost effect
	// In a real implementation, you'd create a custom ghost material
	static ConstructorHelpers::FObjectFinder<UMaterial> GhostMatFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (GhostMatFinder.Succeeded())
	{
		GhostMaterial = GhostMatFinder.Object;
	}
}

FAssetPreviewSystem::~FAssetPreviewSystem()
{
	CleanupPreviewActor();
}

bool FAssetPreviewSystem::StartPreview(
	const FString& AssetPath,
	const FVector& InitialLocation,
	UWorld* World,
	FOnPreviewConfirmed OnConfirmed,
	FOnPreviewCancelled OnCancelled)
{
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("AssetPreviewSystem: Invalid world"));
		return false;
	}

	// Clean up any existing preview
	StopPreview();

	PreviewAssetPath = AssetPath;
	PreviewWorld = World;
	OnConfirmedCallback = OnConfirmed;
	OnCancelledCallback = OnCancelled;
	PreviewScale = FVector::OneVector;
	PreviewRotation = FRotator::ZeroRotator;

	// Create preview actor
	if (!CreatePreviewActor(AssetPath, InitialLocation, World))
	{
		UE_LOG(LogTemp, Error, TEXT("AssetPreviewSystem: Failed to create preview actor"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("AssetPreviewSystem: Preview started for %s"), *AssetPath);
	return true;
}

void FAssetPreviewSystem::StopPreview()
{
	CleanupPreviewActor();
	PreviewWorld = nullptr;
	PreviewAssetPath.Empty();
	OnConfirmedCallback.Unbind();
	OnCancelledCallback.Unbind();
}

void FAssetPreviewSystem::UpdatePreviewLocation(const FVector& NewLocation)
{
	if (PreviewActor)
	{
		FVector ScaledLocation = NewLocation;
		PreviewActor->SetActorLocation(ScaledLocation);
	}
}

void FAssetPreviewSystem::RotatePreview(float DeltaRotation)
{
	if (PreviewActor)
	{
		PreviewRotation.Yaw += DeltaRotation;
		PreviewActor->SetActorRotation(PreviewRotation);
	}
}

void FAssetPreviewSystem::ScalePreview(float ScaleMultiplier)
{
	if (PreviewActor)
	{
		PreviewScale = FVector(ScaleMultiplier);
		PreviewActor->SetActorScale3D(PreviewScale);
	}
}

void FAssetPreviewSystem::ConfirmPreview()
{
	if (PreviewActor && OnConfirmedCallback.IsBound())
	{
		FTransform FinalTransform = PreviewActor->GetActorTransform();
		OnConfirmedCallback.Execute(FinalTransform);
	}

	StopPreview();
}

void FAssetPreviewSystem::CancelPreview()
{
	if (OnCancelledCallback.IsBound())
	{
		OnCancelledCallback.Execute();
	}

	StopPreview();
}

FTransform FAssetPreviewSystem::GetPreviewTransform() const
{
	if (PreviewActor)
	{
		return PreviewActor->GetActorTransform();
	}

	return FTransform::Identity;
}

bool FAssetPreviewSystem::CreatePreviewActor(const FString& AssetPath, const FVector& Location, UWorld* World)
{
	if (!World)
		return false;

	// Load the static mesh
	UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *AssetPath);
	if (!Mesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("AssetPreviewSystem: Could not load mesh: %s"), *AssetPath);
		return false;
	}

	// Spawn a static mesh actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.bTemporaryEditorActor = true; // Mark as temporary

	AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator, SpawnParams);
	if (!MeshActor)
	{
		UE_LOG(LogTemp, Error, TEXT("AssetPreviewSystem: Failed to spawn preview actor"));
		return false;
	}

	// Set the mesh
	UStaticMeshComponent* MeshComponent = MeshActor->GetStaticMeshComponent();
	if (MeshComponent)
	{
		MeshComponent->SetStaticMesh(Mesh);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComponent->SetVisibility(true);

		// Apply ghost material effect
		ApplyGhostMaterial();
	}

	PreviewActor = MeshActor;
	return true;
}

void FAssetPreviewSystem::ApplyGhostMaterial()
{
	if (!PreviewActor)
		return;

	AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(PreviewActor);
	if (!MeshActor)
		return;

	UStaticMeshComponent* MeshComponent = MeshActor->GetStaticMeshComponent();
	if (!MeshComponent)
		return;

	// Create a dynamic material instance with ghost effect
	// This is a simple implementation - a custom material would be better
	int32 NumMaterials = MeshComponent->GetNumMaterials();
	for (int32 i = 0; i < NumMaterials; i++)
	{
		UMaterialInterface* OriginalMaterial = MeshComponent->GetMaterial(i);
		if (OriginalMaterial)
		{
			UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(OriginalMaterial, MeshComponent);
			if (DynMaterial)
			{
				// Make it translucent/ghost-like
				DynMaterial->SetScalarParameterValue(FName("Opacity"), 0.5f);
				MeshComponent->SetMaterial(i, DynMaterial);
			}
		}
	}

	// Set render custom depth for highlighting
	MeshComponent->SetRenderCustomDepth(true);
	MeshComponent->SetCustomDepthStencilValue(255);
}

void FAssetPreviewSystem::CleanupPreviewActor()
{
	if (PreviewActor && PreviewWorld)
	{
		PreviewWorld->DestroyActor(PreviewActor);
		PreviewActor = nullptr;
	}
}
