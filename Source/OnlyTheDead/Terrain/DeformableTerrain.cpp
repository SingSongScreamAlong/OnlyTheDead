#include "Terrain/DeformableTerrain.h"

#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"

ADeformableTerrain::ADeformableTerrain()
{
    PrimaryActorTick.bCanEverTick = false;

    TerrainMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainMesh"));
    TerrainMesh->SetupAttachment(RootComponent);

    // bUseAsyncCooking: rebuilds collision on a background thread after each
    // deformation so the game thread doesn't stall. Set false if you hit
    // issues with rapid back-to-back impacts during a heavy barrage.
    TerrainMesh->bUseAsyncCooking = true;

    // Allow CCD so the player capsule doesn't tunnel through thin rim edges
    TerrainMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    TerrainMesh->SetCollisionResponseToAllChannels(ECR_Block);
}

void ADeformableTerrain::BeginPlay()
{
    Super::BeginPlay();
    BuildGridMesh();
}

// ---------------------------------------------------------------------------
// Grid construction
// ---------------------------------------------------------------------------

void ADeformableTerrain::BuildGridMesh()
{
    GridCols = FMath::RoundToInt(TerrainWidth  / GridResolutionCm) + 1;
    GridRows = FMath::RoundToInt(TerrainDepth  / GridResolutionCm) + 1;

    const int32 TotalVerts = GridCols * GridRows;
    const int32 TotalTris  = (GridCols - 1) * (GridRows - 1) * 6;

    VertexPositions.SetNumUninitialized(TotalVerts);
    Normals.Init(FVector::UpVector, TotalVerts);
    VertexColors.Init(FColor::Black, TotalVerts);   // R=0: undisturbed
    UVs.SetNumUninitialized(TotalVerts);
    Triangles.SetNumUninitialized(TotalTris);

    // Build vertices — flat grid in local XY, Z=0
    for (int32 Row = 0; Row < GridRows; ++Row)
    {
        for (int32 Col = 0; Col < GridCols; ++Col)
        {
            const int32 Idx = GridToIndex(Col, Row);
            VertexPositions[Idx] = FVector(
                Col * GridResolutionCm,
                Row * GridResolutionCm,
                0.0f
            );
            UVs[Idx] = FVector2D(
                (float)Col / (GridCols - 1),
                (float)Row / (GridRows - 1)
            );
        }
    }

    // Build triangle indices (two triangles per quad)
    int32 TriIdx = 0;
    for (int32 Row = 0; Row < GridRows - 1; ++Row)
    {
        for (int32 Col = 0; Col < GridCols - 1; ++Col)
        {
            const int32 BL = GridToIndex(Col,     Row);
            const int32 BR = GridToIndex(Col + 1, Row);
            const int32 TL = GridToIndex(Col,     Row + 1);
            const int32 TR = GridToIndex(Col + 1, Row + 1);

            // Triangle 1 (bottom-left)
            Triangles[TriIdx++] = BL;
            Triangles[TriIdx++] = TL;
            Triangles[TriIdx++] = BR;

            // Triangle 2 (top-right)
            Triangles[TriIdx++] = BR;
            Triangles[TriIdx++] = TL;
            Triangles[TriIdx++] = TR;
        }
    }

    // Create the mesh section (section 0)
    TArray<FLinearColor> LinearColors;
    LinearColors.Reserve(TotalVerts);
    for (const FColor& C : VertexColors)
    {
        LinearColors.Add(FLinearColor(C));
    }

    TerrainMesh->CreateMeshSection_LinearColor(
        0,                // Section index
        VertexPositions,
        Triangles,
        Normals,
        UVs,
        LinearColors,
        TArray<FProcMeshTangent>(),
        true              // Create collision
    );

    if (TerrainMaterial)
    {
        TerrainMesh->SetMaterial(0, TerrainMaterial);
    }

    UE_LOG(LogTemp, Log, TEXT("DeformableTerrain: Built %dx%d grid (%d verts, %.0fm x %.0fm)"),
        GridCols, GridRows, TotalVerts,
        TerrainWidth / 100.0f, TerrainDepth / 100.0f);
}

// ---------------------------------------------------------------------------
// Crater deformation — the main runtime call
// ---------------------------------------------------------------------------

void ADeformableTerrain::ApplyExplosionDeformation(
    FVector WorldImpactPoint,
    float   CraterRadius,
    float   CraterDepth,
    float   RimHeightFraction)
{
    if (VertexPositions.Num() == 0) return;

    // Convert impact to local space
    const FVector LocalImpact = GetActorTransform().InverseTransformPosition(WorldImpactPoint);

    // Influence zone extends to the rim falloff region (1.8x crater radius)
    const float InfluenceRadius = CraterRadius * 1.8f;
    const float RimHeight       = CraterDepth * RimHeightFraction;

    // Determine the range of grid cells to process — avoid iterating all verts
    const int32 ColMin = FMath::Clamp(
        FMath::FloorToInt((LocalImpact.X - InfluenceRadius) / GridResolutionCm), 0, GridCols - 1);
    const int32 ColMax = FMath::Clamp(
        FMath::CeilToInt( (LocalImpact.X + InfluenceRadius) / GridResolutionCm), 0, GridCols - 1);
    const int32 RowMin = FMath::Clamp(
        FMath::FloorToInt((LocalImpact.Y - InfluenceRadius) / GridResolutionCm), 0, GridRows - 1);
    const int32 RowMax = FMath::Clamp(
        FMath::CeilToInt( (LocalImpact.Y + InfluenceRadius) / GridResolutionCm), 0, GridRows - 1);

    bool bAnyVertexChanged = false;

    for (int32 Row = RowMin; Row <= RowMax; ++Row)
    {
        for (int32 Col = ColMin; Col <= ColMax; ++Col)
        {
            const int32 Idx = GridToIndex(Col, Row);
            FVector& Vert = VertexPositions[Idx];

            const float DX = Vert.X - LocalImpact.X;
            const float DY = Vert.Y - LocalImpact.Y;
            const float Dist = FMath::Sqrt(DX * DX + DY * DY);

            if (Dist > InfluenceRadius) continue;

            const float DeltaZ = CraterProfileZ(Dist, CraterRadius, CraterDepth, RimHeight);
            Vert.Z += DeltaZ;

            // Update vertex color: R = cumulative deformation intensity (0-1)
            // Clamp so it never exceeds fully-churned (white)
            const float CurrentIntensity = VertexColors[Idx].R / 255.0f;
            const float AddedIntensity   = FMath::Abs(DeltaZ) / CraterDepth;
            const float NewIntensity     = FMath::Clamp(CurrentIntensity + AddedIntensity, 0.0f, 1.0f);
            VertexColors[Idx].R          = (uint8)(NewIntensity * 255.0f);

            bAnyVertexChanged = true;
        }
    }

    if (!bAnyVertexChanged) return;

    RecalculateNormals();
    UpdateMeshSection();

    UE_LOG(LogTemp, Verbose,
        TEXT("DeformableTerrain: Crater at (%.0f, %.0f) r=%.0fcm depth=%.0fcm [verts %d-%d, %d-%d]"),
        WorldImpactPoint.X, WorldImpactPoint.Y, CraterRadius, CraterDepth,
        ColMin, ColMax, RowMin, RowMax);
}

// ---------------------------------------------------------------------------
// Crater profile function
//
// Defines the Z displacement at distance r from the crater centre.
//
// Three zones:
//   [0, CraterRadius]              — depression bowl (negative Z)
//   [CraterRadius, RimRadius]      — raised rim (positive Z), peaks at ~1.25x CraterRadius
//   [RimRadius, InfluenceRadius]   — smooth falloff back to 0
//
// The bowl shape uses a cosine curve so the centre is smooth (not a spike).
// The rim uses a gaussian falloff so it blends cleanly into the surrounding terrain.
// ---------------------------------------------------------------------------

float ADeformableTerrain::CraterProfileZ(
    float r,
    float CraterRadius,
    float CraterDepth,
    float RimHeight) const
{
    const float RimPeakRadius    = CraterRadius * 1.25f;
    const float RimFalloffRadius = CraterRadius * 1.8f;

    if (r <= CraterRadius)
    {
        // Bowl: cosine profile — deepest at centre, zero at edge
        // cos(π/2 * r/R)² gives a smooth bowl that flattens near the rim
        const float T = r / CraterRadius;
        const float CosVal = FMath::Cos(T * PI * 0.5f);
        return -CraterDepth * (CosVal * CosVal);
    }
    else if (r <= RimFalloffRadius)
    {
        // Rim and falloff: gaussian peak centred at RimPeakRadius
        const float Spread     = (RimFalloffRadius - CraterRadius) * 0.4f;
        const float FromPeak   = r - RimPeakRadius;
        const float Gaussian   = FMath::Exp(-(FromPeak * FromPeak) / (2.0f * Spread * Spread));
        return RimHeight * Gaussian;
    }

    return 0.0f;
}

// ---------------------------------------------------------------------------
// Normal recalculation — cross-product of triangle edges per vertex
// ---------------------------------------------------------------------------

void ADeformableTerrain::RecalculateNormals()
{
    // Reset normals
    for (FVector& N : Normals)
    {
        N = FVector::ZeroVector;
    }

    // Accumulate face normals into each vertex
    for (int32 i = 0; i < Triangles.Num(); i += 3)
    {
        const int32 I0 = Triangles[i];
        const int32 I1 = Triangles[i + 1];
        const int32 I2 = Triangles[i + 2];

        const FVector Edge1 = VertexPositions[I1] - VertexPositions[I0];
        const FVector Edge2 = VertexPositions[I2] - VertexPositions[I0];
        const FVector FaceNormal = FVector::CrossProduct(Edge1, Edge2).GetSafeNormal();

        Normals[I0] += FaceNormal;
        Normals[I1] += FaceNormal;
        Normals[I2] += FaceNormal;
    }

    // Normalise
    for (FVector& N : Normals)
    {
        N = N.GetSafeNormal();
        if (N.IsNearlyZero()) N = FVector::UpVector;
    }
}

// ---------------------------------------------------------------------------
// Push updated geometry to ProceduralMeshComponent
// ---------------------------------------------------------------------------

void ADeformableTerrain::UpdateMeshSection()
{
    TArray<FLinearColor> LinearColors;
    LinearColors.Reserve(VertexColors.Num());
    for (const FColor& C : VertexColors)
    {
        LinearColors.Add(FLinearColor(C));
    }

    // UpdateMeshSection_LinearColor is cheaper than recreating —
    // it only uploads changed vertex buffers and re-cooks collision async
    TerrainMesh->UpdateMeshSection_LinearColor(
        0,
        VertexPositions,
        Normals,
        UVs,
        LinearColors,
        TArray<FProcMeshTangent>()
    );
}

// ---------------------------------------------------------------------------
// Utility
// ---------------------------------------------------------------------------

float ADeformableTerrain::GetDisplacementAtLocation(FVector WorldLocation) const
{
    int32 Col, Row;
    if (!WorldToGrid(WorldLocation, Col, Row)) return 0.0f;

    const int32 Idx = GridToIndex(Col, Row);
    return Idx < VertexPositions.Num() ? VertexPositions[Idx].Z : 0.0f;
}

bool ADeformableTerrain::WorldToGrid(FVector WorldPos, int32& OutCol, int32& OutRow) const
{
    const FVector Local = GetActorTransform().InverseTransformPosition(WorldPos);
    OutCol = FMath::RoundToInt(Local.X / GridResolutionCm);
    OutRow = FMath::RoundToInt(Local.Y / GridResolutionCm);

    return OutCol >= 0 && OutCol < GridCols && OutRow >= 0 && OutRow < GridRows;
}

FVector ADeformableTerrain::GridOrigin() const
{
    return GetActorLocation();
}

void ADeformableTerrain::ResetDeformation()
{
    for (FVector& V : VertexPositions)
    {
        V.Z = 0.0f;
    }
    for (FColor& C : VertexColors)
    {
        C = FColor::Black;
    }
    Normals.Init(FVector::UpVector, Normals.Num());
    UpdateMeshSection();
    UE_LOG(LogTemp, Log, TEXT("DeformableTerrain: Reset to flat ground."));
}

void ADeformableTerrain::RegenerateMesh()
{
    TerrainMesh->ClearAllMeshSections();
    VertexPositions.Empty();
    Normals.Empty();
    VertexColors.Empty();
    UVs.Empty();
    Triangles.Empty();
    BuildGridMesh();
}

// ---------------------------------------------------------------------------
// Editor — regenerate mesh when properties change in Details panel
// ---------------------------------------------------------------------------

#if WITH_EDITOR
void ADeformableTerrain::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    const FName PropName = PropertyChangedEvent.GetPropertyName();
    if (PropName == GET_MEMBER_NAME_CHECKED(ADeformableTerrain, TerrainWidth)  ||
        PropName == GET_MEMBER_NAME_CHECKED(ADeformableTerrain, TerrainDepth)  ||
        PropName == GET_MEMBER_NAME_CHECKED(ADeformableTerrain, GridResolutionCm))
    {
        RegenerateMesh();
    }

    if (PropName == GET_MEMBER_NAME_CHECKED(ADeformableTerrain, TerrainMaterial))
    {
        if (TerrainMaterial)
        {
            TerrainMesh->SetMaterial(0, TerrainMaterial);
        }
    }
}
#endif
