#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeformableTerrain.generated.h"

class UDynamicMeshComponent;

/**
 * ADeformableTerrain  (UE 5.7 — UDynamicMeshComponent)
 *
 * A flat procedural mesh ground plane whose vertices are displaced at runtime
 * by shell impacts. Every deformation is persistent and accumulates — repeated
 * hits deepen craters and widen churned earth, matching the historical reality
 * of the Verdun battlefield where overlapping craters eventually erased all
 * flat ground.
 *
 * Crater profile (cross-section):
 *
 *       RIM     BOWL      RIM
 *      /    \________/    \
 *  ___/      (depth)       \___
 *
 *   - Center depressed by CraterDepth
 *   - Rim raised by RimHeight at RimRadius (1.25x CraterRadius)
 *   - Smooth Gaussian falloff beyond rim back to original terrain
 *   - Vertex colors track cumulative deformation (R: 0=undisturbed, 1=churned)
 *     → drives Substrate material blend between intact ground and churned mud
 *
 * UE 5.7 migration notes (from UProceduralMeshComponent):
 *   - UDynamicMeshComponent uses FDynamicMesh3 internally.
 *   - ApplyExplosionDeformation() calls SetVertex() only on vertices inside the
 *     blast footprint, then NotifyMeshUpdated() — GPU upload is proportional to
 *     crater footprint, not total terrain size. This matters during heavy barrages
 *     (30+ simultaneous craters during Drumfire phase).
 *   - Normals are computed automatically by the component (TangentsType =
 *     AutoCalculated) — RecalculateNormals() is gone.
 *   - Collision uses CTF_UseComplexAsSimple so the player walks into craters.
 *     SetDeferredCollisionUpdates(true) keeps collision cooking off the game thread.
 *
 * Substrate material integration (UE 5.7):
 *   - Assign a Substrate material to TerrainMaterial.
 *   - In the Substrate slab, blend two BSDF layers:
 *       Layer A (undisturbed): chalky limestone, dry, high albedo
 *       Layer B (churned mud):  wet clay, dark, low albedo, SSS approximation
 *     Mix weight = VertexColor.R (0 = A, 1 = B)
 *   - Substrate's multi-lobe BRDF handles the wet/dry specular difference
 *     correctly without manual roughness blending hacks.
 */
UCLASS()
class ONLYTHEDEAD_API ADeformableTerrain : public AActor
{
    GENERATED_BODY()

public:
    ADeformableTerrain();

    // ---- Grid dimensions ----

    // Total terrain width along X (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Grid")
    float TerrainWidth = 20000.0f;      // 200m

    // Total terrain depth along Y (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Grid")
    float TerrainDepth = 10000.0f;      // 100m

    // Distance between vertices (cm). Lower = more crater detail, higher cost.
    // 100cm: good for 75mm craters (4 verts across)
    // 50cm:  excellent detail, ~80k verts for 200x100m terrain
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Grid",
              meta = (ClampMin = "25", ClampMax = "500"))
    float GridResolutionCm = 100.0f;

    // ---- Material ----

    // Assign a Substrate material (UE 5.7) that reads VertexColor.R as the
    // undisturbed → churned blend weight.
    // Legacy UMaterial also works: wire VertexColor.R → any blend parameter.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Material")
    UMaterialInterface* TerrainMaterial;

    // ---- Runtime API (called by ArtilleryShell on detonation) ----

    /**
     * ApplyExplosionDeformation
     *
     * Displaces terrain vertices around WorldImpactPoint to form a crater.
     * Safe to call many times — deformations accumulate additively.
     *
     * Only vertices inside the blast footprint are touched. GPU upload is
     * proportional to crater size, not total terrain vertex count.
     *
     * @param WorldImpactPoint  World-space hit point on terrain surface
     * @param CraterRadius      Radius of the depression bowl (cm)
     * @param CraterDepth       Maximum depth at the crater centre (cm)
     * @param RimHeightFraction Rim height as fraction of CraterDepth (0.30 = 30%)
     */
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void ApplyExplosionDeformation(
        FVector WorldImpactPoint,
        float   CraterRadius,
        float   CraterDepth,
        float   RimHeightFraction = 0.30f
    );

    // Reset all deformation to flat ground (editor utility / level restart)
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain")
    void ResetDeformation();

    // Rebuild mesh from scratch (call after changing grid settings in editor)
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain")
    void RegenerateMesh();

    // Current Z displacement at a world XY position (for audio / stats)
    UFUNCTION(BlueprintPure, Category = "Terrain")
    float GetDisplacementAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintPure, Category = "Terrain")
    int32 GetTotalVertexCount() const { return VertexPositions.Num(); }

protected:
    virtual void BeginPlay() override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
              meta = (AllowPrivateAccess = "true"))
    UDynamicMeshComponent* TerrainMesh;

    // ---- Shadow copies of per-vertex state ----
    // VertexPositions and VertexColors are kept in sync with FDynamicMesh3
    // so callers can read Z displacement and deformation intensity without
    // going through the mesh data structure.

    // Local-space vertex positions — Z is the accumulated displacement
    TArray<FVector> VertexPositions;

    // Per-vertex deformation intensity (R: 0=undisturbed, 1=fully churned)
    // Written to FDynamicMesh3 vertex colors for material use.
    TArray<FColor> VertexColors;

    // FDynamicMesh3 vertex IDs indexed by GridToIndex(Col, Row).
    // For a freshly built mesh these equal the flat index, but storing them
    // explicitly is safe against future mesh editing operations.
    TArray<int32> VertexIDs;

    // Grid dimensions in vertex count
    int32 GridCols = 0;  // Along X
    int32 GridRows = 0;  // Along Y

    // ---- Build / update ----

    void BuildGridMesh();

    // Convert world XY → grid column/row index
    bool WorldToGrid(FVector WorldPos, int32& OutCol, int32& OutRow) const;

    // Convert grid col/row → flat vertex index
    int32 GridToIndex(int32 Col, int32 Row) const { return Row * GridCols + Col; }

    // Crater displacement at distance r from centre
    float CraterProfileZ(float r, float CraterRadius, float CraterDepth, float RimHeight) const;
};
