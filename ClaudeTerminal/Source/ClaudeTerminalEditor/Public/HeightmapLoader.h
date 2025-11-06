// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Loads heightmap data from various file formats
 * Supports: .r16 (16-bit raw), .png (8-bit or 16-bit), .raw (8-bit)
 */
class CLAUDETERMINALEDITOR_API FHeightmapLoader
{
public:
	FHeightmapLoader();
	~FHeightmapLoader();

	/**
	 * Load heightmap from file
	 * @param FilePath Path to heightmap file
	 * @param OutHeightData Output height values (0-65535 range)
	 * @param OutWidth Output width in pixels
	 * @param OutHeight Output height in pixels
	 * @return true if successful
	 */
	bool LoadHeightmap(const FString& FilePath, TArray<uint16>& OutHeightData, int32& OutWidth, int32& OutHeight);

	/**
	 * Load 16-bit raw heightmap (.r16, .raw16)
	 */
	bool LoadR16(const FString& FilePath, TArray<uint16>& OutHeightData, int32& OutWidth, int32& OutHeight);

	/**
	 * Load PNG heightmap (8-bit or 16-bit grayscale)
	 */
	bool LoadPNG(const FString& FilePath, TArray<uint16>& OutHeightData, int32& OutWidth, int32& OutHeight);

	/**
	 * Load 8-bit raw heightmap (.raw, .raw8)
	 */
	bool LoadRaw8(const FString& FilePath, TArray<uint16>& OutHeightData, int32& OutWidth, int32& OutHeight);

	/**
	 * Detect heightmap dimensions from file size (for raw formats)
	 */
	bool DetectDimensions(int64 FileSize, int32 BytesPerPixel, int32& OutWidth, int32& OutHeight);

	/**
	 * Validate heightmap data
	 */
	bool ValidateHeightmap(const TArray<uint16>& HeightData, int32 Width, int32 Height);

	/**
	 * Get heightmap statistics
	 */
	void GetHeightmapStats(const TArray<uint16>& HeightData, uint16& OutMin, uint16& OutMax, float& OutAverage);
};
