#include "Terrain/DeformableTerrain.h"

// UE 5.7 — UDynamicMeshComponent replaces UProceduralMeshComponent.
// FDynamicMesh3 lives in the UE::Geometry namespace since UE 5.5.
#include "DynamicMeshComponent.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "DynamicMesh/DynamicMeshAttributeSet.h"

using namespace UE::Geometry;

ADeformableTerrain::ADeformableTerrain()
{
    PrimaryActorTick.bCanEverTick = false;

    TerrainMesh = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("TerrainMesh"));
    TerrainMesh->SetupAttachment(RootComponent);

    // Complex-as-simple collision: the player capsule walks down into craters
    // because the full mesh is used for physics, not a simplified hull.
    TerrainMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    TerrainMesh->SetCollisionResponseToAllChannels(ECR_Block);
    TerrainMesh->CollisionType = ECollisionTraceFlag::CTF_UseComplexAsSimple;

    // Defer collision cooking to a background thread so the game thread
    // doesn't stall between impacts during a heavy barrage.
    TerrainMesh->SetDeferredCollisionUpdates(true);

    // Auto-compute smooth vertex normals from mesh geometry.
    // Removes the need for a manual RecalculateNormals() pass after each impact.
    TerrainMesh->TangentsType = EDynamicMeshComponentTangentsMode::AutoCalculated;
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

    VertexPositions.SetNumUninitialized(TotalVerts);
    VertexColors.Init(FColor::Black, TotalVerts);  // R=0: undisturbed
    VertexIDs.SetNumUninitialized(TotalVerts);

    FDynamicMesh3* DynMesh = TerrainMesh->GetMesh();
    DynMesh->Clear();

    // Enable per-vertex colours — R channel drives the Substrate mud/dirt blend.
    DynMesh->EnableVertexColors(FVector3f::Zero());

    // Enable UV and normal attribute overlays.
    DynMesh->EnableAttributes();
    DynMesh->Attributes()->SetNumUVLayers(1);

    FDynamicMeshUVOverlay* UV0 = DynMesh->Attributes()->GetUVLayer(0);

    // ---- Append vertices ----
    // One UV element per vertex (no UV seams on a terrain grid).
    TArray<int32> UVElemIDs;
    UVElemIDs.SetNumUninitialized(TotalVerts);

    for (int32 Row = 0; Row < GridRows; ++Row)
    {
        for (int32 Col = 0; Col < GridCols; ++Col)
        {
            const int32 Idx = GridToIndex(Col, Row);

            const FVector3d Pos(
                Col * GridResolutionCm,
                Row * GridResolutionCm,
                0.0
            );
            const int32 VID = DynMesh->AppendVertex(Pos);
            VertexIDs[Idx]      = VID;
            VertexPositions[Idx] = FVector(Pos.X, Pos.Y, Pos.Z);

            DynMesh->SetVertexColor(VID, FVector3f::Zero());  // Undisturbed

            // Normalized [0,1] UV matching the terrain XY extent
            const float U = (float)Col / (GridCols - 1);
            const float V = (float)Row / (GridRows - 1);
            UVElemIDs[Idx] = UV0->AppendElement(FVector2f(U, V));
        }
    }

    // ---- Append triangles (two per grid quad) ----
    for (int32 Row = 0; Row < GridRows - 1; ++Row)
    {
        for (int32 Col = 0; Col < GridCols - 1; ++Col)
        {
            const int32 BL = GridToIndex(Col,     Row);
            const int32 BR = GridToIndex(Col + 1, Row);
            const int32 TL = GridToIndex(Col,     Row + 1);
            const int32 TR = GridToIndex(Col + 1, Row + 1);

            // Triangle 1
            const int32 TID1 = DynMesh->AppendTriangle(
                VertexIDs[BL], VertexIDs[TL], VertexIDs[BR]);
            UV0->SetTriangle(TID1, FIndex3i(UVElemIDs[BL], UVElemIDs[TL], UVElemIDs[BR]));

            // Triangle 2
            const int32 TID2 = DynMesh->AppendTriangle(
                VertexIDs[BR], VertexIDs[TL], VertexIDs[TR]);
            UV0->SetTriangle(TID2, FIndex3i(UVElemIDs[BR], UVElemIDs[TL], UVElemIDs[TR]));
        }
    }

    // Push the built mesh to the render thread and cook initial collision.
    TerrainMesh->NotifyMeshUpdated();

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

    const FVector LocalImpact   = GetActorTransform().InverseTransformPosition(WorldImpactPoint);
    const float InfluenceRadius = CraterRadius * 1.8f;
    const float RimHeight       = CraterDepth * RimHeightFraction;

    // Narrow the iteration to the grid cells overlapping the blast footprint.
    const int32 ColMin = FMath::Clamp(
        FMath::FloorToInt((LocalImpact.X - InfluenceRadius) / GridResolutionCm), 0, GridCols - 1);
    const int32 ColMax = FMath::Clamp(
        FMath::CeilToInt( (LocalImpact.X + InfluenceRadius) / GridResolutionCm), 0, GridCols - 1);
    const int32 RowMin = FMath::Clamp(
        FMath::FloorToInt((LocalImpact.Y - InfluenceRadius) / GridResolutionCm), 0, GridRows - 1);
    const int32 RowMax = FMath::Clamp(
        FMath::CeilToInt( (LocalImpact.Y + InfluenceRadius) / GridResolutionCm), 0, GridRows - 1);

    FDynamicMesh3* DynMesh = TerrainMesh->GetMesh();
    bool bAnyVertexChanged = false;

    for (int32 Row = RowMin; Row <= RowMax; ++Row)
    {
        for (int32 Col = ColMin; Col <= ColMax; ++Col)
        {
            const int32 Idx = GridToIndex(Col, Row);
            FVector& Vert   = VertexPositions[Idx];

            const float DX   = Vert.X - LocalImpact.X;
            const float DY   = Vert.Y - LocalImpact.Y;
            const float Dist = FMath::Sqrt(DX * DX + DY * DY);

            if (Dist > InfluenceRadius) continue;

            const float DeltaZ = CraterProfileZ(Dist, CraterRadius, CraterDepth, RimHeight);
            Vert.Z += DeltaZ;

            // Update deformation intensity in vertex color (R channel).
            const float CurrentIntensity = VertexColors[Idx].R / 255.0f;
            const float AddedIntensity   = FMath::Abs(DeltaZ) / CraterDepth;
            const float NewIntensity     = FMath::Clamp(CurrentIntensity + AddedIntensity, 0.0f, 1.0f);
            VertexColors[Idx].R          = (uint8)(NewIntensity * 255.0f);

            // Write directly into FDynamicMesh3 — only the changed vertices.
            // This is the key perf improvement over UProceduralMeshComponent:
            // SetVertex + SetVertexColor are O(1) per vertex; the GPU upload
            // triggered by NotifyMeshUpdated() sends only the dirty range.
            const int32 VID = VertexIDs[Idx];
            DynMesh->SetVertex(VID, FVector3d(Vert.X, Vert.Y, Vert.Z));
            DynMesh->SetVertexColor(VID, FVector3f(NewIntensity, 0.0f, 0.0f));

            bAnyVertexChanged = true;
        }
    }

    if (!bAnyVertexChanged) return;

    // Normals are recomputed automatically by the component (TangentsType =
    // AutoCalculated) — no manual RecalculateNormals() needed.
    TerrainMesh->NotifyMeshUpdated();

    UE_LOG(LogTemp, Verbose,
        TEXT("DeformableTerrain: Crater at (%.0f, %.0f) r=%.0fcm depth=%.0fcm [cols %d-%d, rows %d-%d]"),
        WorldImpactPoint.X, WorldImpactPoint.Y, CraterRadius, CraterDepth,
        ColMin, ColMax, RowMin, RowMax);
}

// ---------------------------------------------------------------------------
// Crater profile function
//
// Three zones:
//   [0, CraterRadius]              — bowl (negative Z, cosine profile)
//   [CraterRadius, RimRadius]      — raised rim (positive Z, Gaussian peak)
//   [RimRadius, InfluenceRadius]   — smooth falloff back to 0
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
        // Bowl: cosine² — smooth at centre, zero at rim edge
        const float T      = r / CraterRadius;
        const float CosVal = FMath::Cos(T * PI * 0.5f);
        return -CraterDepth * (CosVal * CosVal);
    }
    else if (r <= RimFalloffRadius)
    {
        // Rim: Gaussian centred at RimPeakRadius
        const float Spread   = (RimFalloffRadius - CraterRadius) * 0.4f;
        const float FromPeak = r - RimPeakRadius;
        return RimHeight * FMath::Exp(-(FromPeak * FromPeak) / (2.0f * Spread * Spread));
    }

    return 0.0f;
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

void ADeformableTerrain::ResetDeformation()
{
    FDynamicMesh3* DynMesh = TerrainMesh->GetMesh();

    for (int32 Idx = 0; Idx < VertexPositions.Num(); ++Idx)
    {
        VertexPositions[Idx].Z = 0.0f;
        VertexColors[Idx]      = FColor::Black;

        const int32 VID = VertexIDs[Idx];
        const FVector3d OldPos = DynMesh->GetVertex(VID);
        DynMesh->SetVertex(VID, FVector3d(OldPos.X, OldPos.Y, 0.0));
        DynMesh->SetVertexColor(VID, FVector3f::Zero());
    }

    TerrainMesh->NotifyMeshUpdated();
    UE_LOG(LogTemp, Log, TEXT("DeformableTerrain: Reset to flat ground."));
}

void ADeformableTerrain::RegenerateMesh()
{
    VertexPositions.Empty();
    VertexColors.Empty();
    VertexIDs.Empty();
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

    if (PropName == GET_MEMBER_NAME_CHECKED(ADeformableTerrain, TerrainMaterial) &&
        TerrainMaterial)
    {
        TerrainMesh->SetMaterial(0, TerrainMaterial);
    }
}
#endif
