// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Converts between GPS coordinates (WGS84) and UE5 world coordinates
 * Specialized for Verdun battlefield area (49.1°N - 49.3°N, 5.3°E - 5.5°E)
 */
class CLAUDETERMINALEDITOR_API FGPSCoordinateConverter
{
public:
	FGPSCoordinateConverter();
	~FGPSCoordinateConverter();

	/**
	 * Set the reference point (origin) for GPS conversion
	 * @param Latitude Reference latitude in degrees
	 * @param Longitude Reference longitude in degrees
	 * @param Altitude Reference altitude in meters (defaults to 0)
	 */
	void SetReferencePoint(double Latitude, double Longitude, double Altitude = 0.0);

	/**
	 * Set Verdun battlefield as reference point
	 * Uses Fort Douaumont as origin: 49.1597°N, 5.4267°E, 365m elevation
	 */
	void SetVerdunReference();

	/**
	 * Convert GPS coordinates to UE5 world coordinates
	 * @param Latitude Latitude in degrees
	 * @param Longitude Longitude in degrees
	 * @param Altitude Altitude in meters above sea level
	 * @return UE5 world position (in cm)
	 */
	FVector GPSToUE5(double Latitude, double Longitude, double Altitude) const;

	/**
	 * Convert UE5 world coordinates to GPS coordinates
	 * @param WorldPosition UE5 world position (in cm)
	 * @param OutLatitude Output latitude in degrees
	 * @param OutLongitude Output longitude in degrees
	 * @param OutAltitude Output altitude in meters
	 */
	void UE5ToGPS(const FVector& WorldPosition, double& OutLatitude, double& OutLongitude, double& OutAltitude) const;

	/**
	 * Calculate distance between two GPS points (in meters)
	 */
	double CalculateDistance(double Lat1, double Lon1, double Lat2, double Lon2) const;

	/**
	 * Get scale factor (meters per degree) at current reference latitude
	 * @param OutMetersPerDegreeLat Meters per degree of latitude
	 * @param OutMetersPerDegreeLon Meters per degree of longitude
	 */
	void GetScaleFactors(double& OutMetersPerDegreeLat, double& OutMetersPerDegreeLon) const;

	/**
	 * Validate GPS coordinates are within reasonable range
	 */
	bool ValidateGPSCoordinates(double Latitude, double Longitude) const;

	/**
	 * Get Verdun battlefield bounds
	 */
	void GetVerdunBounds(double& OutMinLat, double& OutMaxLat, double& OutMinLon, double& OutMaxLon) const;

private:
	// Reference point (origin)
	double RefLatitude;
	double RefLongitude;
	double RefAltitude;

	// Earth radius in meters
	static constexpr double EarthRadiusMeters = 6371000.0;

	// Convert degrees to radians
	double DegreesToRadians(double Degrees) const { return Degrees * PI / 180.0; }

	// Convert radians to degrees
	double RadiansToDegrees(double Radians) const { return Radians * 180.0 / PI; }

	// Calculate meters per degree at given latitude
	void CalculateScaleFactors(double Latitude, double& OutMetersPerDegreeLat, double& OutMetersPerDegreeLon) const;
};
