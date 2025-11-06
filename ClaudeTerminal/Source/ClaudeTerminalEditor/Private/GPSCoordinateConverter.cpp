// Copyright Epic Games, Inc. All Rights Reserved.

#include "GPSCoordinateConverter.h"

FGPSCoordinateConverter::FGPSCoordinateConverter()
	: RefLatitude(0.0)
	, RefLongitude(0.0)
	, RefAltitude(0.0)
{
	// Default to Verdun
	SetVerdunReference();
}

FGPSCoordinateConverter::~FGPSCoordinateConverter()
{
}

void FGPSCoordinateConverter::SetReferencePoint(double Latitude, double Longitude, double Altitude)
{
	RefLatitude = Latitude;
	RefLongitude = Longitude;
	RefAltitude = Altitude;

	UE_LOG(LogTemp, Log, TEXT("GPS reference point set: %.6f°N, %.6f°E, %.1fm"), Latitude, Longitude, Altitude);
}

void FGPSCoordinateConverter::SetVerdunReference()
{
	// Fort Douaumont coordinates
	RefLatitude = 49.1597;
	RefLongitude = 5.4267;
	RefAltitude = 365.0; // meters above sea level

	UE_LOG(LogTemp, Log, TEXT("Verdun reference set: Fort Douaumont (49.1597°N, 5.4267°E, 365m)"));
}

FVector FGPSCoordinateConverter::GPSToUE5(double Latitude, double Longitude, double Altitude) const
{
	// Calculate offset from reference point
	double DeltaLat = Latitude - RefLatitude;
	double DeltaLon = Longitude - RefLongitude;
	double DeltaAlt = Altitude - RefAltitude;

	// Get scale factors at reference latitude
	double MetersPerDegreeLat, MetersPerDegreeLon;
	CalculateScaleFactors(RefLatitude, MetersPerDegreeLat, MetersPerDegreeLon);

	// Convert to meters
	double OffsetX = DeltaLon * MetersPerDegreeLon;  // East-West
	double OffsetY = DeltaLat * MetersPerDegreeLat;  // North-South
	double OffsetZ = DeltaAlt;                        // Altitude

	// Convert meters to UE5 centimeters
	// UE5 axes: X=Forward(North), Y=Right(East), Z=Up
	FVector WorldPosition;
	WorldPosition.X = OffsetY * 100.0;  // North-South → X
	WorldPosition.Y = OffsetX * 100.0;  // East-West → Y
	WorldPosition.Z = OffsetZ * 100.0;  // Altitude → Z

	return WorldPosition;
}

void FGPSCoordinateConverter::UE5ToGPS(const FVector& WorldPosition, double& OutLatitude, double& OutLongitude, double& OutAltitude) const
{
	// Convert UE5 centimeters to meters
	double OffsetY = WorldPosition.X / 100.0;  // X → North-South
	double OffsetX = WorldPosition.Y / 100.0;  // Y → East-West
	double OffsetZ = WorldPosition.Z / 100.0;  // Z → Altitude

	// Get scale factors
	double MetersPerDegreeLat, MetersPerDegreeLon;
	CalculateScaleFactors(RefLatitude, MetersPerDegreeLat, MetersPerDegreeLon);

	// Convert meters to degrees
	double DeltaLat = OffsetY / MetersPerDegreeLat;
	double DeltaLon = OffsetX / MetersPerDegreeLon;

	// Add to reference point
	OutLatitude = RefLatitude + DeltaLat;
	OutLongitude = RefLongitude + DeltaLon;
	OutAltitude = RefAltitude + OffsetZ;
}

double FGPSCoordinateConverter::CalculateDistance(double Lat1, double Lon1, double Lat2, double Lon2) const
{
	// Haversine formula
	double dLat = DegreesToRadians(Lat2 - Lat1);
	double dLon = DegreesToRadians(Lon2 - Lon1);

	double a = FMath::Sin(dLat / 2.0) * FMath::Sin(dLat / 2.0) +
	           FMath::Cos(DegreesToRadians(Lat1)) * FMath::Cos(DegreesToRadians(Lat2)) *
	           FMath::Sin(dLon / 2.0) * FMath::Sin(dLon / 2.0);

	double c = 2.0 * FMath::Atan2(FMath::Sqrt(a), FMath::Sqrt(1.0 - a));
	double distance = EarthRadiusMeters * c;

	return distance;
}

void FGPSCoordinateConverter::CalculateScaleFactors(double Latitude, double& OutMetersPerDegreeLat, double& OutMetersPerDegreeLon) const
{
	double LatRad = DegreesToRadians(Latitude);

	// Meters per degree of latitude (constant)
	OutMetersPerDegreeLat = 111132.954 - 559.822 * FMath::Cos(2.0 * LatRad) + 1.175 * FMath::Cos(4.0 * LatRad);

	// Meters per degree of longitude (varies with latitude)
	OutMetersPerDegreeLon = 111412.84 * FMath::Cos(LatRad) - 93.5 * FMath::Cos(3.0 * LatRad);
}

void FGPSCoordinateConverter::GetScaleFactors(double& OutMetersPerDegreeLat, double& OutMetersPerDegreeLon) const
{
	CalculateScaleFactors(RefLatitude, OutMetersPerDegreeLat, OutMetersPerDegreeLon);
}

bool FGPSCoordinateConverter::ValidateGPSCoordinates(double Latitude, double Longitude) const
{
	// Check valid ranges
	if (Latitude < -90.0 || Latitude > 90.0)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid latitude: %.6f (must be -90 to 90)"), Latitude);
		return false;
	}

	if (Longitude < -180.0 || Longitude > 180.0)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid longitude: %.6f (must be -180 to 180)"), Longitude);
		return false;
	}

	// Check if reasonably close to Verdun (within ~500km)
	double Distance = CalculateDistance(Latitude, Longitude, RefLatitude, RefLongitude);
	if (Distance > 500000.0) // 500km
	{
		UE_LOG(LogTemp, Warning, TEXT("GPS coordinates very far from reference (%.1f km)"), Distance / 1000.0);
	}

	return true;
}

void FGPSCoordinateConverter::GetVerdunBounds(double& OutMinLat, double& OutMaxLat, double& OutMinLon, double& OutMaxLon) const
{
	// Verdun battlefield approximate bounds (60km x 60km area)
	OutMinLat = 49.10;  // Southern edge
	OutMaxLat = 49.30;  // Northern edge
	OutMinLon = 5.30;   // Western edge
	OutMaxLon = 5.55;   // Eastern edge
}
