// Copyright Only The Dead Development Team. All Rights Reserved.

#include "VerdunContentGenerator.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "Landscape.h"
#include "KismetProceduralMeshLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"
#include "Engine/StaticMesh.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "TerrainSculptor.h"
#include "Components/SplineMeshComponent.h"

UVerdunContentGenerator::UVerdunContentGenerator()
{
}

AActor* UVerdunContentGenerator::GenerateTrenchSystem(
	UWorld* World,
	const TArray<FVector>& SplinePath,
	const FTrenchConfiguration& Config)
{
	if (!World || SplinePath.Num() < 2)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid world or path for trench generation"));
		return nullptr;
	}

	// Create the trench actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATrenchSystemActor* TrenchActor = World->SpawnActor<ATrenchSystemActor>(
		ATrenchSystemActor::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!TrenchActor)
	{
		return nullptr;
	}

	TrenchActor->Configuration = Config;
	TrenchActor->bIsFrenchTrench = true;

	// Generate zigzag path if needed
	TArray<FVector> FinalPath = Config.bZigzag
		? GenerateZigzagPath(SplinePath, 300.0f) // 3m zigzag distance
		: SplinePath;

	// Create the trench mesh
	CreateTrenchMesh(World, TrenchActor, FinalPath, Config);

	// Add dugouts
	if (Config.DugoutSpacing > 0.0f)
	{
		int32 DugoutCount = Config.DugoutCount;
		if (DugoutCount == 0)
		{
			// Auto-calculate based on spacing
			float TotalLength = 0.0f;
			for (int32 i = 1; i < FinalPath.Num(); i++)
			{
				TotalLength += FVector::Dist(FinalPath[i - 1], FinalPath[i]);
			}
			DugoutCount = FMath::FloorToInt(TotalLength / Config.DugoutSpacing);
		}

		// Place dugouts along the path
		for (int32 i = 0; i < DugoutCount; i++)
		{
			float T = (float)(i + 1) / (float)(DugoutCount + 1);
			int32 Index = FMath::FloorToInt(T * (FinalPath.Num() - 1));
			Index = FMath::Clamp(Index, 0, FinalPath.Num() - 2);

			FVector Location = FMath::Lerp(FinalPath[Index], FinalPath[Index + 1], FMath::Frac(T * (FinalPath.Num() - 1)));
			FVector Direction = (FinalPath[Index + 1] - FinalPath[Index]).GetSafeNormal();
			FRotator Rotation = Direction.Rotation();

			AddDugout(World, TrenchActor, Location, Rotation);
		}
	}

	// Add sandbags, duckboards, etc.
	if (Config.bAddSandbags)
	{
		AddSandbags(World, TrenchActor, FinalPath);
	}

	if (Config.bAddDuckboards)
	{
		AddDuckboards(World, TrenchActor, FinalPath);
	}

	// Sculpt terrain
	SculptTerrainForTrench(World, FinalPath, Config.Width, Config.Depth);

	TrenchActor->SetActorLabel(TEXT("French Trench System"));

	UE_LOG(LogTemp, Log, TEXT("Generated French trench system with %d dugouts"), Config.DugoutCount);

	return TrenchActor;
}

AActor* UVerdunContentGenerator::GenerateGermanTrenchSystem(
	UWorld* World,
	const TArray<FVector>& SplinePath,
	const FTrenchConfiguration& Config)
{
	// German trenches were typically deeper and more fortified
	FTrenchConfiguration GermanConfig = Config;
	GermanConfig.Depth = 250.0f; // 2.5m deep (German trenches were deeper)
	GermanConfig.bAddDuckboards = true; // Germans used more duckboards

	AActor* TrenchActor = GenerateTrenchSystem(World, SplinePath, GermanConfig);

	if (TrenchActor)
	{
		TrenchActor->SetActorLabel(TEXT("German Trench System"));
		if (ATrenchSystemActor* TrenchSystem = Cast<ATrenchSystemActor>(TrenchActor))
		{
			TrenchSystem->bIsFrenchTrench = false;
		}
	}

	return TrenchActor;
}

AActor* UVerdunContentGenerator::CreateArtilleryCrater(
	UWorld* World,
	const FVector& Location,
	const FArtilleryCraterConfig& Config)
{
	if (!World)
	{
		return nullptr;
	}

	// Create a simple crater actor
	FActorSpawnParameters SpawnParams;
	AActor* CraterActor = World->SpawnActor<AActor>(
		AActor::StaticClass(),
		Location,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!CraterActor)
	{
		return nullptr;
	}

	CraterActor->SetActorLabel(TEXT("Artillery Crater"));

	// TODO: Use ProceduralMeshComponent or GeometryScripting to create actual crater geometry
	// For now, we'll just create a placeholder

	UE_LOG(LogTemp, Log, TEXT("Created artillery crater at %s (diameter: %.1fm)"),
		*Location.ToString(), Config.Diameter / 100.0f);

	return CraterActor;
}

void UVerdunContentGenerator::GenerateNoMansLand(
	UWorld* World,
	const TArray<FVector>& FrenchTrenchLine,
	const TArray<FVector>& GermanTrenchLine,
	int32 CraterCount,
	bool bAddBarbedWire)
{
	if (!World || FrenchTrenchLine.Num() == 0 || GermanTrenchLine.Num() == 0)
	{
		return;
	}

	// Calculate the area between trenches
	FVector FrenchCenter = FVector::ZeroVector;
	for (const FVector& Point : FrenchTrenchLine)
	{
		FrenchCenter += Point;
	}
	FrenchCenter /= FrenchTrenchLine.Num();

	FVector GermanCenter = FVector::ZeroVector;
	for (const FVector& Point : GermanTrenchLine)
	{
		GermanCenter += Point;
	}
	GermanCenter /= GermanTrenchLine.Num();

	// Generate random craters
	FRandomStream RandomStream(FMath::Rand());
	for (int32 i = 0; i < CraterCount; i++)
	{
		float T = RandomStream.FRand();
		FVector CraterLocation = FMath::Lerp(FrenchCenter, GermanCenter, T);

		// Add some random offset
		CraterLocation.X += RandomStream.FRandRange(-500.0f, 500.0f);
		CraterLocation.Y += RandomStream.FRandRange(-500.0f, 500.0f);

		FArtilleryCraterConfig CraterConfig;
		CraterConfig.Diameter = RandomStream.FRandRange(500.0f, 1000.0f);
		CraterConfig.bFillWithWater = RandomStream.FRand() > 0.7f; // 30% chance of water

		CreateArtilleryCrater(World, CraterLocation, CraterConfig);
	}

	// Add barbed wire
	if (bAddBarbedWire)
	{
		GenerateBarbedWire(World, FrenchTrenchLine, 3, 80.0f); // 3 rows, 0.8m high
	}

	UE_LOG(LogTemp, Log, TEXT("Generated No Man's Land with %d craters"), CraterCount);
}

AActor* UVerdunContentGenerator::CreateFortification(
	UWorld* World,
	const FVector& Location,
	const FRotator& Rotation,
	const FFortificationConfig& Config)
{
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	AActor* FortActor = World->SpawnActor<AActor>(
		AActor::StaticClass(),
		Location,
		Rotation,
		SpawnParams
	);

	if (!FortActor)
	{
		return nullptr;
	}

	FortActor->SetActorLabel(FString::Printf(TEXT("Fort %s"), *Config.FortType));

	// TODO: Add fort geometry based on type
	// This would include concrete walls, gun emplacements, etc.

	UE_LOG(LogTemp, Log, TEXT("Created fortification: %s"), *Config.FortType);

	return FortActor;
}

void UVerdunContentGenerator::GenerateDestroyedVillage(
	UWorld* World,
	const FVector& CenterLocation,
	float Radius,
	int32 BuildingCount,
	float DestructionLevel)
{
	if (!World)
	{
		return;
	}

	FRandomStream RandomStream(FMath::Rand());

	for (int32 i = 0; i < BuildingCount; i++)
	{
		// Random position within radius
		float Angle = RandomStream.FRandRange(0.0f, 2.0f * PI);
		float Distance = RandomStream.FRandRange(0.0f, Radius);

		FVector BuildingLocation = CenterLocation;
		BuildingLocation.X += FMath::Cos(Angle) * Distance;
		BuildingLocation.Y += FMath::Sin(Angle) * Distance;

		// TODO: Spawn destroyed building meshes

		UE_LOG(LogTemp, Log, TEXT("Placed destroyed building at %s"), *BuildingLocation.ToString());
	}

	UE_LOG(LogTemp, Log, TEXT("Generated destroyed village with %d buildings"), BuildingCount);
}

AActor* UVerdunContentGenerator::CreateCommunicationTrench(
	UWorld* World,
	const FVector& StartPoint,
	const FVector& EndPoint,
	const FTrenchConfiguration& Config)
{
	// Communication trenches were narrower and less fortified
	FTrenchConfiguration CommConfig = Config;
	CommConfig.Width = 100.0f; // 1m wide
	CommConfig.bAddDugouts = false;
	CommConfig.bAddFiringSteps = false;

	TArray<FVector> Path;
	Path.Add(StartPoint);
	Path.Add(EndPoint);

	AActor* CommTrench = GenerateTrenchSystem(World, Path, CommConfig);

	if (CommTrench)
	{
		CommTrench->SetActorLabel(TEXT("Communication Trench"));
	}

	return CommTrench;
}

void UVerdunContentGenerator::GenerateBarbedWire(
	UWorld* World,
	const TArray<FVector>& Path,
	int32 Rows,
	float Height)
{
	if (!World || Path.Num() < 2)
	{
		return;
	}

	// Create container actor for all barbed wire
	FActorSpawnParameters SpawnParams;
	AActor* WireContainer = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (!WireContainer)
	{
		return;
	}
	WireContainer->SetActorLabel(TEXT("Barbed Wire Obstacle"));

	// Create instanced mesh component for fence posts
	UInstancedStaticMeshComponent* PostISM = NewObject<UInstancedStaticMeshComponent>(WireContainer);
	PostISM->RegisterComponent();
	WireContainer->SetRootComponent(PostISM);

	// Load fence post mesh (fallback to cylinder if not found)
	UStaticMesh* PostMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
	if (PostMesh)
	{
		PostISM->SetStaticMesh(PostMesh);
	}

	// Generate wire obstacles along path
	float PostSpacing = 200.0f; // Posts every 2 meters
	float RowSpacing = 30.0f;   // 30cm between rows

	for (int32 Row = 0; Row < Rows; Row++)
	{
		float RowHeight = Height + (Row * RowSpacing);

		// Place posts along the path
		for (int32 i = 0; i < Path.Num() - 1; i++)
		{
			FVector Start = Path[i];
			FVector End = Path[i + 1];
			Start.Z += RowHeight;
			End.Z += RowHeight;

			float Distance = FVector::Dist(Start, End);
			int32 PostCount = FMath::CeilToInt(Distance / PostSpacing);

			for (int32 j = 0; j <= PostCount; j++)
			{
				float T = (float)j / (float)PostCount;
				FVector PostPosition = FMath::Lerp(Start, End, T);

				// Create fence post transform
				FTransform PostTransform;
				PostTransform.SetLocation(PostPosition);
				PostTransform.SetRotation(FQuat::Identity);
				PostTransform.SetScale3D(FVector(0.1f, 0.1f, RowHeight / 100.0f)); // Thin post, height scaled

				PostISM->AddInstance(PostTransform);
			}

			// Create wire between posts using procedural mesh
			if (Row == 0) // Only create wire geometry for first row (others implied)
			{
				UProceduralMeshComponent* WireMesh = NewObject<UProceduralMeshComponent>(WireContainer);
				WireMesh->RegisterComponent();
				WireMesh->AttachToComponent(WireContainer->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

				// Create simple wire geometry (thin tube along path)
				TArray<FVector> Vertices;
				TArray<int32> Triangles;
				TArray<FVector> Normals;
				TArray<FVector2D> UV0;

				float WireRadius = 2.0f; // 2cm wire radius

				// Create wire mesh (simplified - just a line of quads)
				FVector Direction = (End - Start).GetSafeNormal();
				FVector Right = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();
				FVector Up = FVector::CrossProduct(Right, Direction).GetSafeNormal();

				// 4 vertices per cross-section
				Vertices.Add(Start + Right * WireRadius);
				Vertices.Add(Start - Right * WireRadius);
				Vertices.Add(End + Right * WireRadius);
				Vertices.Add(End - Right * WireRadius);

				// Create quad
				Triangles.Append({0, 2, 1, 1, 2, 3});

				// Simple normals
				for (int32 k = 0; k < 4; k++)
				{
					Normals.Add(Up);
					UV0.Add(FVector2D(0, 0));
				}

				WireMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, TArray<FColor>(), TArray<FProcMeshTangent>(), false);
			}
		}
	}

	int32 TotalPosts = PostISM->GetInstanceCount();
	UE_LOG(LogTemp, Log, TEXT("Generated barbed wire obstacle: %d rows, %d posts"), Rows, TotalPosts);
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

TArray<FVector> UVerdunContentGenerator::GenerateZigzagPath(const TArray<FVector>& BasePath, float ZigzagDistance)
{
	if (BasePath.Num() < 2)
	{
		return BasePath;
	}

	TArray<FVector> ZigzagPath;
	ZigzagPath.Add(BasePath[0]); // Start point

	bool bZigRight = true;

	for (int32 i = 1; i < BasePath.Num(); i++)
	{
		FVector Start = BasePath[i - 1];
		FVector End = BasePath[i];
		FVector Direction = (End - Start).GetSafeNormal();
		FVector Right = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();

		float SegmentLength = FVector::Dist(Start, End);
		int32 ZigzagCount = FMath::FloorToInt(SegmentLength / 1000.0f); // Zigzag every 10m

		for (int32 j = 1; j <= ZigzagCount; j++)
		{
			float T = (float)j / (float)(ZigzagCount + 1);
			FVector MidPoint = FMath::Lerp(Start, End, T);

			FVector Offset = Right * ZigzagDistance * (bZigRight ? 1.0f : -1.0f);
			ZigzagPath.Add(MidPoint + Offset);

			bZigRight = !bZigRight;
		}

		ZigzagPath.Add(End);
	}

	return ZigzagPath;
}

void UVerdunContentGenerator::CreateTrenchMesh(
	UWorld* World,
	AActor* TrenchActor,
	const TArray<FVector>& Path,
	const FTrenchConfiguration& Config)
{
	if (!TrenchActor || Path.Num() < 2)
	{
		return;
	}

	// Create procedural mesh component
	UProceduralMeshComponent* ProceduralMesh = NewObject<UProceduralMeshComponent>(TrenchActor);
	ProceduralMesh->RegisterComponent();
	TrenchActor->SetRootComponent(ProceduralMesh);

	// Build trench geometry
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	float HalfWidth = Config.Width / 2.0f;
	float Depth = Config.Depth;

	// Generate mesh along path
	for (int32 i = 0; i < Path.Num() - 1; i++)
	{
		FVector Start = Path[i];
		FVector End = Path[i + 1];
		FVector Direction = (End - Start).GetSafeNormal();
		FVector Right = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();

		// Current segment vertices (8 vertices per segment cross-section)
		int32 BaseIndex = Vertices.Num();

		// Left wall top
		Vertices.Add(Start + Right * HalfWidth);
		// Left wall bottom
		Vertices.Add(Start + Right * HalfWidth - FVector(0, 0, Depth));
		// Floor left
		Vertices.Add(Start + Right * (Config.FloorWidth / 2.0f) - FVector(0, 0, Depth));
		// Floor center left
		Vertices.Add(Start - FVector(0, 0, Depth));
		// Floor center right
		Vertices.Add(Start - FVector(0, 0, Depth));
		// Floor right
		Vertices.Add(Start - Right * (Config.FloorWidth / 2.0f) - FVector(0, 0, Depth));
		// Right wall bottom
		Vertices.Add(Start - Right * HalfWidth - FVector(0, 0, Depth));
		// Right wall top
		Vertices.Add(Start - Right * HalfWidth);

		// Next segment vertices
		if (i < Path.Num() - 2)
		{
			// Add UVs and normals
			for (int32 j = 0; j < 8; j++)
			{
				UV0.Add(FVector2D((float)i, (float)j / 7.0f));
				Normals.Add(FVector::UpVector);
			}

			// Create triangles connecting this segment to next
			// Left wall
			Triangles.Add(BaseIndex + 0);
			Triangles.Add(BaseIndex + 8);
			Triangles.Add(BaseIndex + 1);
			Triangles.Add(BaseIndex + 1);
			Triangles.Add(BaseIndex + 8);
			Triangles.Add(BaseIndex + 9);

			// Left slope
			Triangles.Add(BaseIndex + 1);
			Triangles.Add(BaseIndex + 9);
			Triangles.Add(BaseIndex + 2);
			Triangles.Add(BaseIndex + 2);
			Triangles.Add(BaseIndex + 9);
			Triangles.Add(BaseIndex + 10);

			// Floor
			Triangles.Add(BaseIndex + 3);
			Triangles.Add(BaseIndex + 11);
			Triangles.Add(BaseIndex + 4);
			Triangles.Add(BaseIndex + 4);
			Triangles.Add(BaseIndex + 11);
			Triangles.Add(BaseIndex + 12);

			// Right slope
			Triangles.Add(BaseIndex + 5);
			Triangles.Add(BaseIndex + 13);
			Triangles.Add(BaseIndex + 6);
			Triangles.Add(BaseIndex + 6);
			Triangles.Add(BaseIndex + 13);
			Triangles.Add(BaseIndex + 14);

			// Right wall
			Triangles.Add(BaseIndex + 6);
			Triangles.Add(BaseIndex + 14);
			Triangles.Add(BaseIndex + 7);
			Triangles.Add(BaseIndex + 7);
			Triangles.Add(BaseIndex + 14);
			Triangles.Add(BaseIndex + 15);
		}
	}

	// Create the mesh
	ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, VertexColors, Tangents, true);

	// Try to load and apply trench material
	UMaterialInterface* TrenchMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (TrenchMaterial)
	{
		ProceduralMesh->SetMaterial(0, TrenchMaterial);
	}

	UE_LOG(LogTemp, Log, TEXT("Created procedural trench mesh with %d vertices, %d triangles"), Vertices.Num(), Triangles.Num() / 3);
}

void UVerdunContentGenerator::AddDugout(UWorld* World, AActor* ParentActor, const FVector& Location, const FRotator& Rotation)
{
	if (!World || !ParentActor)
	{
		return;
	}

	// Create dugout entrance (simple box for now - could be replaced with proper mesh)
	UProceduralMeshComponent* DugoutMesh = NewObject<UProceduralMeshComponent>(ParentActor);
	DugoutMesh->RegisterComponent();
	DugoutMesh->AttachToComponent(ParentActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	DugoutMesh->SetRelativeLocation(Location);
	DugoutMesh->SetRelativeRotation(Rotation);

	// Create simple dugout entrance geometry (2m wide, 2m tall, 3m deep)
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;

	float Width = 200.0f;  // 2m
	float Height = 200.0f; // 2m
	float Depth = 300.0f;  // 3m

	// Front face vertices (entrance)
	Vertices.Add(FVector(0, -Width/2, 0));
	Vertices.Add(FVector(0, Width/2, 0));
	Vertices.Add(FVector(0, Width/2, Height));
	Vertices.Add(FVector(0, -Width/2, Height));

	// Back face vertices
	Vertices.Add(FVector(Depth, -Width/2, 0));
	Vertices.Add(FVector(Depth, Width/2, 0));
	Vertices.Add(FVector(Depth, Width/2, Height));
	Vertices.Add(FVector(Depth, -Width/2, Height));

	// Create triangles for dugout box
	// Front (entrance)
	Triangles.Append({0, 2, 1, 0, 3, 2});
	// Back
	Triangles.Append({4, 5, 6, 4, 6, 7});
	// Left
	Triangles.Append({0, 4, 7, 0, 7, 3});
	// Right
	Triangles.Append({1, 2, 6, 1, 6, 5});
	// Top
	Triangles.Append({3, 7, 6, 3, 6, 2});
	// Bottom
	Triangles.Append({0, 1, 5, 0, 5, 4});

	// Generate normals
	for (int32 i = 0; i < Vertices.Num(); i++)
	{
		Normals.Add(FVector::UpVector);
		UV0.Add(FVector2D(0, 0));
	}

	DugoutMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, TArray<FColor>(), TArray<FProcMeshTangent>(), true);

	UE_LOG(LogTemp, Log, TEXT("Added dugout entrance at %s"), *Location.ToString());
}

void UVerdunContentGenerator::AddSandbags(UWorld* World, AActor* ParentActor, const TArray<FVector>& Path)
{
	if (!World || !ParentActor || Path.Num() < 2)
	{
		return;
	}

	// Create instanced static mesh component for efficient rendering
	UInstancedStaticMeshComponent* SandbagISM = NewObject<UInstancedStaticMeshComponent>(ParentActor);
	SandbagISM->RegisterComponent();
	SandbagISM->AttachToComponent(ParentActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	// Try to load sandbag mesh (fallback to cube if not found)
	UStaticMesh* SandbagMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
	if (SandbagMesh)
	{
		SandbagISM->SetStaticMesh(SandbagMesh);
	}

	// Place sandbags along path
	float SandbagSpacing = 50.0f; // 0.5m between sandbags

	for (int32 i = 0; i < Path.Num() - 1; i++)
	{
		FVector Start = Path[i];
		FVector End = Path[i + 1];
		float Distance = FVector::Dist(Start, End);
		int32 BagCount = FMath::FloorToInt(Distance / SandbagSpacing);

		for (int32 j = 0; j <= BagCount; j++)
		{
			float T = (float)j / (float)(BagCount + 1);
			FVector Position = FMath::Lerp(Start, End, T);

			// Offset to side for parapet
			FVector Direction = (End - Start).GetSafeNormal();
			FVector Right = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();
			Position += Right * 100.0f; // 1m offset
			Position.Z += 50.0f; // 0.5m up

			// Create transform
			FTransform Transform;
			Transform.SetLocation(Position);
			Transform.SetRotation(Direction.ToOrientationQuat());
			Transform.SetScale3D(FVector(0.5f, 0.8f, 0.4f)); // Sandbag-like dimensions

			SandbagISM->AddInstance(Transform);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Added %d sandbag instances along trench"), SandbagISM->GetInstanceCount());
}

void UVerdunContentGenerator::AddDuckboards(UWorld* World, AActor* ParentActor, const TArray<FVector>& Path)
{
	if (!World || !ParentActor || Path.Num() < 2)
	{
		return;
	}

	// Create instanced static mesh component for duckboards
	UInstancedStaticMeshComponent* DuckboardISM = NewObject<UInstancedStaticMeshComponent>(ParentActor);
	DuckboardISM->RegisterComponent();
	DuckboardISM->AttachToComponent(ParentActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	// Use a simple plane/cube for duckboard (could be replaced with proper wooden plank mesh)
	UStaticMesh* DuckboardMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
	if (DuckboardMesh)
	{
		DuckboardISM->SetStaticMesh(DuckboardMesh);
	}

	// Place duckboard planks along trench floor
	float PlankLength = 100.0f; // 1m planks
	float PlankWidth = 20.0f;   // 20cm wide
	float PlankSpacing = 5.0f;  // 5cm gap between planks

	for (int32 i = 0; i < Path.Num() - 1; i++)
	{
		FVector Start = Path[i];
		FVector End = Path[i + 1];
		Start.Z -= 200.0f; // Place on trench floor (2m down)
		End.Z -= 200.0f;

		FVector Direction = (End - Start).GetSafeNormal();
		FVector Right = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();

		float Distance = FVector::Dist(Start, End);
		int32 PlankCount = FMath::FloorToInt(Distance / (PlankLength + PlankSpacing));

		for (int32 j = 0; j <= PlankCount; j++)
		{
			float T = (float)j / (float)(PlankCount + 1);
			FVector Position = FMath::Lerp(Start, End, T);

			// Create transform for duckboard plank
			FTransform Transform;
			Transform.SetLocation(Position);
			Transform.SetRotation(Right.ToOrientationQuat()); // Orient perpendicular to trench
			Transform.SetScale3D(FVector(PlankLength / 100.0f, PlankWidth / 100.0f, 0.05f));

			DuckboardISM->AddInstance(Transform);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Added %d duckboard plank instances"), DuckboardISM->GetInstanceCount());
}

void UVerdunContentGenerator::SculptTerrainForTrench(UWorld* World, const TArray<FVector>& Path, float Width, float Depth)
{
	// Use TerrainSculptor to actually excavate the trench
	FTrenchExcavationParams Params;
	Params.Path = Path;
	Params.Width = Width;
	Params.Depth = Depth;
	Params.WallAngle = 80.0f;
	Params.FloorWidth = Width * 0.6f; // 60% of width for flat floor
	Params.bCreateParapet = true;
	Params.ParapetHeight = 50.0f;

	FString Result;
	bool bSuccess = UTerrainSculptor::ExcavateTrench(World, Params, Result);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Sculpted terrain for trench: %s"), *Result);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to sculpt terrain: %s"), *Result);
	}
}

// ============================================================================
// TRENCH SYSTEM ACTOR
// ============================================================================

ATrenchSystemActor::ATrenchSystemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	TrenchPath = CreateDefaultSubobject<USplineComponent>(TEXT("TrenchPath"));
	RootComponent = TrenchPath;
}

void ATrenchSystemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Rebuild trench when properties change in editor
	// This allows for real-time preview
}
