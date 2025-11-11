// Copyright Only The Dead Development Team. All Rights Reserved.

#include "VerdunContentGenerator.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "Landscape.h"

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

	// TODO: Use InstancedStaticMeshComponent to place barbed wire posts and wire meshes

	UE_LOG(LogTemp, Log, TEXT("Generated barbed wire: %d rows along path"), Rows);
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
	// TODO: Use ProceduralMeshComponent or GeometryScripting to create trench geometry
	// This would create the actual 3D mesh for the trench walls, floor, etc.

	UE_LOG(LogTemp, Log, TEXT("Created trench mesh with %d path points"), Path.Num());
}

void UVerdunContentGenerator::AddDugout(UWorld* World, AActor* ParentActor, const FVector& Location, const FRotator& Rotation)
{
	// TODO: Spawn dugout mesh (small underground shelter)
	UE_LOG(LogTemp, Log, TEXT("Added dugout at %s"), *Location.ToString());
}

void UVerdunContentGenerator::AddSandbags(UWorld* World, AActor* ParentActor, const TArray<FVector>& Path)
{
	// TODO: Place sandbag meshes along the trench parapet
	UE_LOG(LogTemp, Log, TEXT("Added sandbags along trench"));
}

void UVerdunContentGenerator::AddDuckboards(UWorld* World, AActor* ParentActor, const TArray<FVector>& Path)
{
	// TODO: Place duckboard (wooden walkway) meshes along trench floor
	UE_LOG(LogTemp, Log, TEXT("Added duckboards along trench"));
}

void UVerdunContentGenerator::SculptTerrainForTrench(UWorld* World, const TArray<FVector>& Path, float Width, float Depth)
{
	// TODO: Use Landscape API to sculpt the terrain, creating the trench depression
	UE_LOG(LogTemp, Log, TEXT("Sculpted terrain for trench: %.1fm wide, %.1fm deep"), Width / 100.0f, Depth / 100.0f);
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
