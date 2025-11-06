// Copyright Epic Games, Inc. All Rights Reserved.

#include "HeightmapLoader.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"

FHeightmapLoader::FHeightmapLoader()
{
}

FHeightmapLoader::~FHeightmapLoader()
{
}

bool FHeightmapLoader::LoadHeightmap(const FString& FilePath, TArray<uint16>& OutHeightData, int32& OutWidth, int32& OutHeight)
{
	if (!FPaths::FileExists(FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Heightmap file not found: %s"), *FilePath);
		return false;
	}

	FString Extension = FPaths::GetExtension(FilePath).ToLower();

	if (Extension == TEXT("r16") || Extension == TEXT("raw16"))
	{
		return LoadR16(FilePath, OutHeightData, OutWidth, OutHeight);
	}
	else if (Extension == TEXT("png"))
	{
		return LoadPNG(FilePath, OutHeightData, OutWidth, OutHeight);
	}
	else if (Extension == TEXT("raw") || Extension == TEXT("raw8"))
	{
		return LoadRaw8(FilePath, OutHeightData, OutWidth, OutHeight);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Unsupported heightmap format: %s"), *Extension);
		return false;
	}
}

bool FHeightmapLoader::LoadR16(const FString& FilePath, TArray<uint16>& OutHeightData, int32& OutWidth, int32& OutHeight)
{
	// Load raw binary file
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load R16 file: %s"), *FilePath);
		return false;
	}

	// Detect dimensions (assuming square or common sizes)
	int64 FileSize = FileData.Num();
	if (!DetectDimensions(FileSize, 2, OutWidth, OutHeight))
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot detect R16 dimensions from file size: %lld"), FileSize);
		return false;
	}

	// Convert bytes to uint16 (big-endian typically)
	int32 NumPixels = OutWidth * OutHeight;
	OutHeightData.SetNum(NumPixels);

	for (int32 i = 0; i < NumPixels; i++)
	{
		// Read as big-endian (SRTM format)
		uint8 High = FileData[i * 2];
		uint8 Low = FileData[i * 2 + 1];
		OutHeightData[i] = (static_cast<uint16>(High) << 8) | Low;
	}

	UE_LOG(LogTemp, Log, TEXT("Loaded R16 heightmap: %dx%d pixels"), OutWidth, OutHeight);
	return true;
}

bool FHeightmapLoader::LoadPNG(const FString& FilePath, TArray<uint16>& OutHeightData, int32& OutWidth, int32& OutHeight)
{
	// Load PNG file
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load PNG file: %s"), *FilePath);
		return false;
	}

	// Use image wrapper to decode PNG
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to decode PNG: %s"), *FilePath);
		return false;
	}

	OutWidth = ImageWrapper->GetWidth();
	OutHeight = ImageWrapper->GetHeight();
	int32 BitDepth = ImageWrapper->GetBitDepth();

	TArray<uint8> RawData;
	if (!ImageWrapper->GetRaw(ERGBFormat::Gray, BitDepth, RawData))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to extract PNG raw data: %s"), *FilePath);
		return false;
	}

	int32 NumPixels = OutWidth * OutHeight;
	OutHeightData.SetNum(NumPixels);

	if (BitDepth == 16)
	{
		// 16-bit PNG
		for (int32 i = 0; i < NumPixels; i++)
		{
			uint8 High = RawData[i * 2];
			uint8 Low = RawData[i * 2 + 1];
			OutHeightData[i] = (static_cast<uint16>(High) << 8) | Low;
		}
	}
	else
	{
		// 8-bit PNG - scale up to 16-bit
		for (int32 i = 0; i < NumPixels; i++)
		{
			OutHeightData[i] = static_cast<uint16>(RawData[i]) * 257; // 0-255 -> 0-65535
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Loaded PNG heightmap: %dx%d pixels (%d-bit)"), OutWidth, OutHeight, BitDepth);
	return true;
}

bool FHeightmapLoader::LoadRaw8(const FString& FilePath, TArray<uint16>& OutHeightData, int32& OutWidth, int32& OutHeight)
{
	// Load raw binary file
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load RAW file: %s"), *FilePath);
		return false;
	}

	// Detect dimensions
	int64 FileSize = FileData.Num();
	if (!DetectDimensions(FileSize, 1, OutWidth, OutHeight))
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot detect RAW8 dimensions from file size: %lld"), FileSize);
		return false;
	}

	// Convert 8-bit to 16-bit
	int32 NumPixels = OutWidth * OutHeight;
	OutHeightData.SetNum(NumPixels);

	for (int32 i = 0; i < NumPixels; i++)
	{
		OutHeightData[i] = static_cast<uint16>(FileData[i]) * 257; // 0-255 -> 0-65535
	}

	UE_LOG(LogTemp, Log, TEXT("Loaded RAW8 heightmap: %dx%d pixels"), OutWidth, OutHeight);
	return true;
}

bool FHeightmapLoader::DetectDimensions(int64 FileSize, int32 BytesPerPixel, int32& OutWidth, int32& OutHeight)
{
	int64 NumPixels = FileSize / BytesPerPixel;

	// Try common sizes (square)
	int32 CommonSizes[] = {
		256, 512, 1024, 2048, 4096, 8192,
		1201, 3601, 7201, // SRTM common sizes
		2017, 4033, 8065  // UE5 landscape sizes
	};

	for (int32 Size : CommonSizes)
	{
		if (NumPixels == Size * Size)
		{
			OutWidth = Size;
			OutHeight = Size;
			return true;
		}
	}

	// Try rectangular (common aspect ratios)
	TArray<TPair<int32, int32>> CommonRects = {
		{1201, 1201}, {3601, 3601},
		{2048, 1024}, {4096, 2048},
		{8192, 4096}, {8192, 8192}
	};

	for (const auto& Rect : CommonRects)
	{
		if (NumPixels == Rect.Key * Rect.Value)
		{
			OutWidth = Rect.Key;
			OutHeight = Rect.Value;
			return true;
		}
	}

	// Assume square
	int32 SqrtSize = FMath::RoundToInt(FMath::Sqrt(static_cast<float>(NumPixels)));
	if (SqrtSize * SqrtSize == NumPixels)
	{
		OutWidth = SqrtSize;
		OutHeight = SqrtSize;
		UE_LOG(LogTemp, Warning, TEXT("Detected square heightmap: %dx%d (might be incorrect)"), OutWidth, OutHeight);
		return true;
	}

	return false;
}

bool FHeightmapLoader::ValidateHeightmap(const TArray<uint16>& HeightData, int32 Width, int32 Height)
{
	if (HeightData.Num() != Width * Height)
	{
		UE_LOG(LogTemp, Error, TEXT("Heightmap data size mismatch: expected %d, got %d"), Width * Height, HeightData.Num());
		return false;
	}

	if (Width < 32 || Height < 32)
	{
		UE_LOG(LogTemp, Error, TEXT("Heightmap too small: %dx%d (minimum 32x32)"), Width, Height);
		return false;
	}

	if (Width > 8192 || Height > 8192)
	{
		UE_LOG(LogTemp, Warning, TEXT("Heightmap very large: %dx%d (may cause performance issues)"), Width, Height);
	}

	return true;
}

void FHeightmapLoader::GetHeightmapStats(const TArray<uint16>& HeightData, uint16& OutMin, uint16& OutMax, float& OutAverage)
{
	if (HeightData.Num() == 0)
	{
		OutMin = 0;
		OutMax = 0;
		OutAverage = 0.0f;
		return;
	}

	OutMin = TNumericLimits<uint16>::Max();
	OutMax = 0;
	uint64 Sum = 0;

	for (uint16 Value : HeightData)
	{
		OutMin = FMath::Min(OutMin, Value);
		OutMax = FMath::Max(OutMax, Value);
		Sum += Value;
	}

	OutAverage = static_cast<float>(Sum) / HeightData.Num();

	UE_LOG(LogTemp, Log, TEXT("Heightmap stats: Min=%d, Max=%d, Avg=%.1f, Range=%d"),
		OutMin, OutMax, OutAverage, OutMax - OutMin);
}
