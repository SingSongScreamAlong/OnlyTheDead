// Verdun World Builder Plugin
// Terrain deformation implementation - RUNTIME DESTRUCTIBLE TERRAIN

#include "VerdunTerrainDeformation.h"
#include "LandscapeInfo.h"
#include "LandscapeDataAccess.h"
#include "LandscapeEdit.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UVerdunTerrainDeformation::UVerdunTerrainDeformation()
{
	PrimaryComponentTick.bCanEverTick = true;

	TargetLandscape = nullptr;
	bAutoFindLandscape = true;
	bEnableRuntimeDeformation = true;
	bBatchDeformations = true;
	MaxDeformationsPerFrame = 10;
	bSmoothEdges = true;
	bUpdateCollision = true;

	TotalDeformationsApplied = 0;
	bHasOriginalData = false;
}

void UVerdunTerrainDeformation::BeginPlay()
{
	Super::BeginPlay();

	// Auto-find landscape
	if (bAutoFindLandscape && !TargetLandscape)
	{
		for (TActorIterator<ALandscape> It(GetWorld()); It; ++It)
		{
			TargetLandscape = *It;
			UE_LOG(LogTemp, Log, TEXT("VerdunTerrainDeformation: Auto-found landscape: %s"), *TargetLandscape->GetName());
			break;
		}
	}

	if (!TargetLandscape)
	{
		UE_LOG(LogTemp, Warning, TEXT("VerdunTerrainDeformation: No landscape found!"));
	}
}

void UVerdunTerrainDeformation::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Process pending deformations if batching
	if (bBatchDeformations && PendingDeformations.Num() > 0)
	{
		ProcessPendingDeformations();
	}
}

void UVerdunTerrainDeformation::DeformCrater(FVector Location, float Radius, float Depth, EVerdunDeformationFalloff Falloff, bool bImmediate)
{
	FVerdunDeformation Deformation;
	Deformation.Location = Location;
	Deformation.Radius = Radius;
	Deformation.Depth = Depth;
	Deformation.Shape = EVerdunDeformationShape::Crater;
	Deformation.Falloff = Falloff;

	ApplyDeformation(Deformation, bImmediate);
}

void UVerdunTerrainDeformation::DeformTrench(FVector StartLocation, FVector EndLocation, float Width, float Depth, bool bZigzag)
{
	FVerdunDeformation Deformation;
	Deformation.Location = StartLocation;
	Deformation.Radius = Width;
	Deformation.Depth = Depth;
	Deformation.Shape = EVerdunDeformationShape::Trench;
	Deformation.Direction = (EndLocation - StartLocation).GetSafeNormal();
	Deformation.Length = FVector::Dist(StartLocation, EndLocation);

	ApplyDeformation(Deformation, false);
}

void UVerdunTerrainDeformation::ApplyDeformation(const FVerdunDeformation& Deformation, bool bImmediate)
{
	if (!bEnableRuntimeDeformation)
	{
		UE_LOG(LogTemp, Warning, TEXT("Runtime deformation is disabled"));
		return;
	}

	if (!TargetLandscape)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot deform: No target landscape"));
		return;
	}

	if (bImmediate || !bBatchDeformations)
	{
		ApplyDeformationInternal(Deformation);
	}
	else
	{
		PendingDeformations.Add(Deformation);
	}
}

void UVerdunTerrainDeformation::BatchApplyDeformations(const TArray<FVerdunDeformation>& Deformations)
{
	PendingDeformations.Append(Deformations);
}

void UVerdunTerrainDeformation::ProcessPendingDeformations()
{
	if (PendingDeformations.Num() == 0)
	{
		return;
	}

	int32 NumToProcess = FMath::Min(MaxDeformationsPerFrame, PendingDeformations.Num());

	for (int32 i = 0; i < NumToProcess; i++)
	{
		ApplyDeformationInternal(PendingDeformations[i]);
	}

	// Remove processed
	PendingDeformations.RemoveAt(0, NumToProcess);

	if (NumToProcess > 0)
	{
		OnBatchDeformationsComplete(NumToProcess);
	}
}

void UVerdunTerrainDeformation::ClearPendingDeformations()
{
	PendingDeformations.Empty();
	UE_LOG(LogTemp, Log, TEXT("Cleared pending deformations"));
}

void UVerdunTerrainDeformation::ResetTerrain()
{
	// TODO: Implement terrain reset using saved original data
	UE_LOG(LogTemp, Warning, TEXT("ResetTerrain not yet implemented"));
}

bool UVerdunTerrainDeformation::GetTerrainHeightAtLocation(FVector Location, float& OutHeight)
{
	if (!TargetLandscape)
	{
		return false;
	}

	// Query landscape height
	FVector HitLocation;
	if (TargetLandscape->GetHeightAtLocation(Location, &HitLocation.Z))
	{
		OutHeight = HitLocation.Z;
		return true;
	}

	return false;
}

void UVerdunTerrainDeformation::ApplyDeformationInternal(const FVerdunDeformation& Deformation)
{
	switch (Deformation.Shape)
	{
		case EVerdunDeformationShape::Crater:
			ApplyCraterDeformation(Deformation);
			break;

		case EVerdunDeformationShape::Trench:
			ApplyTrenchDeformation(Deformation);
			break;

		default:
			UE_LOG(LogTemp, Warning, TEXT("Unsupported deformation shape"));
			break;
	}
}

void UVerdunTerrainDeformation::ApplyCraterDeformation(const FVerdunDeformation& Deformation)
{
	if (!TargetLandscape)
	{
		return;
	}

	ULandscapeInfo* LandscapeInfo = TargetLandscape->GetLandscapeInfo();
	if (!LandscapeInfo)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot get landscape info"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Applying crater deformation at %s (R:%.0f D:%.0f)"),
		*Deformation.Location.ToString(), Deformation.Radius, Deformation.Depth);

	// Get landscape edit interface
	// NOTE: FLandscapeEditDataInterface is the key to runtime landscape editing
	FLandscapeEditDataInterface LandscapeEdit(LandscapeInfo);

	// Convert world location to landscape coordinates
	FVector LandscapeLocation = TargetLandscape->GetTransform().InverseTransformPosition(Deformation.Location);

	// Calculate bounds
	float RadiusInLandscapeUnits = Deformation.Radius / TargetLandscape->GetActorScale3D().X;
	int32 MinX = FMath::FloorToInt((LandscapeLocation.X - RadiusInLandscapeUnits) / 100.0f);
	int32 MinY = FMath::FloorToInt((LandscapeLocation.Y - RadiusInLandscapeUnits) / 100.0f);
	int32 MaxX = FMath::CeilToInt((LandscapeLocation.X + RadiusInLandscapeUnits) / 100.0f);
	int32 MaxY = FMath::CeilToInt((LandscapeLocation.Y + RadiusInLandscapeUnits) / 100.0f);

	// Iterate over affected area
	for (int32 Y = MinY; Y <= MaxY; Y++)
	{
		for (int32 X = MinX; X <= MaxX; X++)
		{
			FVector VertexWorldPos = TargetLandscape->GetTransform().TransformPosition(FVector(X * 100.0f, Y * 100.0f, 0.0f));
			float Distance = FVector::Dist2D(VertexWorldPos, Deformation.Location);

			if (Distance <= Deformation.Radius)
			{
				// Calculate falloff
				float Falloff = CalculateFalloff(Distance, Deformation.Radius, Deformation.Falloff);

				// Get current height
				uint16 CurrentHeight = LandscapeEdit.GetHeight(X, Y);

				// Calculate new height
				float HeightChange = -Deformation.Depth * Falloff; // Negative = crater
				uint16 NewHeight = FMath::Clamp(CurrentHeight + (int32)HeightChange, 0, 65535);

				// Set new height
				LandscapeEdit.SetHeight(X, Y, NewHeight);
			}
		}
	}

	// Flush changes to landscape
	LandscapeEdit.Flush();

	// Update collision if needed
	if (bUpdateCollision)
	{
		TargetLandscape->RecreateCollisionComponents();
	}

	TotalDeformationsApplied++;
	OnDeformationApplied(Deformation);

	UE_LOG(LogTemp, Log, TEXT("Crater deformation applied (%d total)"), TotalDeformationsApplied);
}

void UVerdunTerrainDeformation::ApplyTrenchDeformation(const FVerdunDeformation& Deformation)
{
	if (!TargetLandscape)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Applying trench deformation from %s (L:%.0f W:%.0f D:%.0f)"),
		*Deformation.Location.ToString(), Deformation.Length, Deformation.Radius, Deformation.Depth);

	// Sample points along trench path
	int32 NumSamples = FMath::CeilToInt(Deformation.Length / 100.0f); // Sample every 1m

	for (int32 i = 0; i <= NumSamples; i++)
	{
		float Alpha = (float)i / (float)NumSamples;
		FVector SampleLocation = Deformation.Location + (Deformation.Direction * Deformation.Length * Alpha);

		// Apply crater at each sample point (trenches are like connected craters)
		FVerdunDeformation CraterDef;
		CraterDef.Location = SampleLocation;
		CraterDef.Radius = Deformation.Radius; // Width
		CraterDef.Depth = Deformation.Depth;
		CraterDef.Shape = EVerdunDeformationShape::Crater;
		CraterDef.Falloff = EVerdunDeformationFalloff::Sharp; // Sharp edges for trenches

		ApplyCraterDeformation(CraterDef);
	}

	UE_LOG(LogTemp, Log, TEXT("Trench deformation complete"));
}

float UVerdunTerrainDeformation::CalculateFalloff(float Distance, float Radius, EVerdunDeformationFalloff FalloffType)
{
	float NormalizedDistance = FMath::Clamp(Distance / Radius, 0.0f, 1.0f);

	switch (FalloffType)
	{
		case EVerdunDeformationFalloff::Linear:
			return 1.0f - NormalizedDistance;

		case EVerdunDeformationFalloff::Smooth:
			// Cosine falloff for smooth crater edges
			return FMath::Cos(NormalizedDistance * PI * 0.5f);

		case EVerdunDeformationFalloff::Sharp:
			// Exponential falloff for sharp edges
			return FMath::Pow(1.0f - NormalizedDistance, 3.0f);

		default:
			return 1.0f - NormalizedDistance;
	}
}
