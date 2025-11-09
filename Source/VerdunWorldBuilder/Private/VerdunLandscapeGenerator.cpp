// Verdun World Builder Plugin
// Landscape generator implementation

#include "VerdunLandscapeGenerator.h"
#include "LandscapeStreamingProxy.h"
#include "LandscapeInfo.h"
#include "LandscapeDataAccess.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "Misc/FileHelper.h"
#include "Math/UnrealMathUtility.h"

AVerdunLandscapeGenerator::AVerdunLandscapeGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	SetupLandscapeDefaults();
}

void AVerdunLandscapeGenerator::SetupLandscapeDefaults()
{
	// Default settings for Verdun (60 km² battlefield)
	LandscapeQuadsPerSection = 255; // 256 verts per section
	LandscapeSectionsPerComponent = 1;
	LandscapeComponentsX = 8;
	LandscapeComponentsY = 8;

	// Scale: 30m SRTM data = 3000cm per vertex
	LandscapeScale = FVector(3000.0f, 3000.0f, 100.0f); // X, Y in cm, Z = height scale

	// Verdun elevations
	MinElevationMeters = 165.0f;
	MaxElevationMeters = 390.0f;

	// Procedural settings
	ProceduralNoiseScale = 1000.0f;
	ProceduralNoiseStrength = 50.0f;
	ProceduralSeed = 12345;

	bAutoGenerateOnBeginPlay = false;
	bUseProcedural = false;

	HeightmapSizeX = 0;
	HeightmapSizeY = 0;
}

void AVerdunLandscapeGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoGenerateOnBeginPlay)
	{
		GenerateLandscape(!bUseProcedural);
	}
}

bool AVerdunLandscapeGenerator::GenerateLandscape(bool bFromHeightmap)
{
	UE_LOG(LogTemp, Log, TEXT("=== GENERATING VERDUN LANDSCAPE ==="));

	TArray<uint16> LocalHeightData;
	int32 SizeX = 0, SizeY = 0;

	if (bFromHeightmap && !HeightmapFilePath.IsEmpty())
	{
		// Load from file
		if (!LoadHeightmapFromFile(HeightmapFilePath, LocalHeightData, SizeX, SizeY))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load heightmap from: %s"), *HeightmapFilePath);
			OnLandscapeGenerationFailed(TEXT("Failed to load heightmap file"));
			return false;
		}

		UE_LOG(LogTemp, Log, TEXT("Loaded heightmap: %dx%d from %s"), SizeX, SizeY, *HeightmapFilePath);
	}
	else
	{
		// Generate procedurally
		SizeX = (LandscapeQuadsPerSection * LandscapeSectionsPerComponent * LandscapeComponentsX) + 1;
		SizeY = (LandscapeQuadsPerSection * LandscapeSectionsPerComponent * LandscapeComponentsY) + 1;

		UE_LOG(LogTemp, Log, TEXT("Generating procedural heightmap: %dx%d"), SizeX, SizeY);
		GenerateProceduralHeightmap(SizeX, SizeY, LocalHeightData);
	}

	// Store height data
	HeightData = LocalHeightData;
	HeightmapSizeX = SizeX;
	HeightmapSizeY = SizeY;

	// Create landscape
	GeneratedLandscape = CreateLandscapeFromHeightData(HeightData, HeightmapSizeX, HeightmapSizeY);

	if (GeneratedLandscape)
	{
		UE_LOG(LogTemp, Log, TEXT("Landscape generated successfully!"));
		UE_LOG(LogTemp, Log, TEXT("  Size: %dx%d vertices"), HeightmapSizeX, HeightmapSizeY);
		UE_LOG(LogTemp, Log, TEXT("  Scale: %.0f x %.0f x %.0f"), LandscapeScale.X, LandscapeScale.Y, LandscapeScale.Z);
		UE_LOG(LogTemp, Log, TEXT("  Elevation: %.0fm - %.0fm"), MinElevationMeters, MaxElevationMeters);

		OnLandscapeGenerated(GeneratedLandscape);
		return true;
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to create landscape"));
	OnLandscapeGenerationFailed(TEXT("Failed to create landscape actor"));
	return false;
}

bool AVerdunLandscapeGenerator::LoadHeightmapFromFile(const FString& FilePath, TArray<uint16>& OutHeightData, int32& OutSizeX, int32& OutSizeY)
{
	// Load file data
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read file: %s"), *FilePath);
		return false;
	}

	// Get image wrapper module
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

	// Detect format (PNG expected)
	EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(FileData.GetData(), FileData.Num());

	if (ImageFormat == EImageFormat::Invalid)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid image format"));
		return false;
	}

	// Create image wrapper
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);

	if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to decompress image"));
		return false;
	}

	// Get dimensions
	OutSizeX = ImageWrapper->GetWidth();
	OutSizeY = ImageWrapper->GetHeight();

	// Get raw data
	TArray<uint8> RawData;
	if (!ImageWrapper->GetRaw(ERGBFormat::Gray, 16, RawData))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get raw image data"));
		return false;
	}

	// Convert to uint16 array
	OutHeightData.SetNum(OutSizeX * OutSizeY);
	FMemory::Memcpy(OutHeightData.GetData(), RawData.GetData(), OutSizeX * OutSizeY * sizeof(uint16));

	UE_LOG(LogTemp, Log, TEXT("Loaded %dx%d heightmap (%d KB)"), OutSizeX, OutSizeY, (OutHeightData.Num() * 2) / 1024);

	return true;
}

void AVerdunLandscapeGenerator::GenerateProceduralHeightmap(int32 SizeX, int32 SizeY, TArray<uint16>& OutHeightData)
{
	OutHeightData.SetNum(SizeX * SizeY);

	FRandomStream Random(ProceduralSeed);

	// Simple Perlin-style noise (simplified for this example)
	for (int32 Y = 0; Y < SizeY; Y++)
	{
		for (int32 X = 0; X < SizeX; X++)
		{
			float NoiseX = X / ProceduralNoiseScale;
			float NoiseY = Y / ProceduralNoiseScale;

			// Multi-octave noise (simplified - real implementation would use proper Perlin noise)
			float Height = 0.0f;
			float Amplitude = 1.0f;
			float Frequency = 1.0f;

			for (int32 Octave = 0; Octave < 4; Octave++)
			{
				Height += FMath::Sin(NoiseX * Frequency * PI) * FMath::Cos(NoiseY * Frequency * PI) * Amplitude;
				Frequency *= 2.0f;
				Amplitude *= 0.5f;
			}

			// Normalize to 0-1 range
			Height = (Height + 1.0f) * 0.5f;

			// Add some randomness
			Height += Random.FRandRange(-0.1f, 0.1f);
			Height = FMath::Clamp(Height, 0.0f, 1.0f);

			// Convert to uint16 (0-65535)
			uint16 HeightValue = FMath::RoundToInt(Height * 65535.0f);

			int32 Index = Y * SizeX + X;
			OutHeightData[Index] = HeightValue;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Generated procedural heightmap: %dx%d"), SizeX, SizeY);
}

ALandscape* AVerdunLandscapeGenerator::CreateLandscapeFromHeightData(const TArray<uint16>& HeightDataArray, int32 SizeX, int32 SizeY)
{
	UE_LOG(LogTemp, Log, TEXT("Creating landscape from height data..."));

	if (HeightDataArray.Num() == 0 || SizeX == 0 || SizeY == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid height data"));
		return nullptr;
	}

	// Spawn landscape actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(TEXT("VerdunLandscape"));
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FTransform LandscapeTransform;
	LandscapeTransform.SetLocation(GetActorLocation());

	ALandscape* Landscape = GetWorld()->SpawnActor<ALandscape>(ALandscape::StaticClass(), LandscapeTransform, SpawnParams);

	if (!Landscape)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn landscape actor"));
		return nullptr;
	}

	// Configure landscape
	Landscape->SetActorScale3D(FVector(LandscapeScale.X / 100.0f, LandscapeScale.Y / 100.0f, LandscapeScale.Z / 100.0f));
	Landscape->bCastStaticShadow = true;

	UE_LOG(LogTemp, Log, TEXT("Landscape actor created: %s"), *Landscape->GetName());

	// NOTE: Actual heightmap import requires using FLandscapeEditorUtils::Import
	// which is editor-only. For runtime, you would need to use a different approach
	// or pre-generate the landscape in the editor.

	UE_LOG(LogTemp, Warning, TEXT("Heightmap import is editor-only. For runtime generation:"));
	UE_LOG(LogTemp, Warning, TEXT("  1. Use this tool in editor to generate landscape"));
	UE_LOG(LogTemp, Warning, TEXT("  2. Or use pre-built landscape and only deform at runtime"));

	return Landscape;
}

bool AVerdunLandscapeGenerator::GetHeightAtPosition(FVector WorldPosition, float& OutHeight)
{
	if (!GeneratedLandscape)
	{
		return false;
	}

	// Use landscape's built-in height query
	FVector LocalPosition = GeneratedLandscape->GetTransform().InverseTransformPosition(WorldPosition);

	// This is simplified - real implementation would use FLandscapeEditDataInterface
	OutHeight = 0.0f;

	return true;
}
