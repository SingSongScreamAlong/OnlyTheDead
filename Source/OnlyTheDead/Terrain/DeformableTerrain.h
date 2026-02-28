#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeformableTerrain.generated.h"

class UProceduralMeshComponent;

/**
 * ADeformableTerrain
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
 *   - Rim raised by RimHeight at RimRadius (1.4x CraterRadius)
 *   - Smooth falloff beyond rim back to original terrain
 *   - Vertex colors track cumulative deformation intensity (0=undisturbed,
 *     1=fully churned) — drives material mud/dirt blending
 *
 * Collision updates automatically after every deformation so the player
 * physically walks down into craters rather than floating above them.
 *
 * Performance:
 *   - Grid resolution is configurable; 100cm default gives good crater
 *     fidelity for 75mm shells (200cm crater radius = 4 vertices across)
 *   - Only vertices within the blast footprint are processed per impact
 *   - Async cooking recommended for large terrain (see bCookAsync)
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

    // Assign a material that reads VertexColor.R as deformation intensity
    // (0 = undisturbed soil, 1 = fully churned/crater).
    // Blend between intact ground texture and churned mud based on this value.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Material")
    UMaterialInterface* TerrainMaterial;

    // ---- Runtime API (called by ArtilleryShell on detonation) ----

    /**
     * ApplyExplosionDeformation
     *
     * Displaces terrain vertices around WorldImpactPoint to form a crater.
     * Safe to call many times — deformations accumulate additively.
     *
     * @param WorldImpactPoint  World-space hit point on terrain surface
     * @param CraterRadius      Radius of the depression bowl (cm)
     * @param CraterDepth       Maximum depth at the crater centre (cm)
     * @param RimHeightFraction Rim height as fraction of CraterDepth (0.25 = 25%)
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

    // Rebuild the mesh from scratch (call after changing grid settings in editor)
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain")
    void RegenerateMesh();

    // Read the current displacement at a world XY position (for audio / VFX)
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
    UProceduralMeshComponent* TerrainMesh;

    // ---- Grid state ----

    // Vertex positions in local space — Z is the accumulated displacement
    TArray<FVector> VertexPositions;

    // Vertex normals (recalculated after each deformation)
    TArray<FVector> Normals;

    // Vertex colors — R channel = cumulative deformation intensity (0-1)
    // Used by terrain material to blend mud/dirt textures
    TArray<FColor> VertexColors;

    // UV coordinates (static after mesh creation)
    TArray<FVector2D> UVs;

    // Triangle indices (static after mesh creation)
    TArray<int32> Triangles;

    // Grid dimensions in vertex count
    int32 GridCols = 0;  // Along X
    int32 GridRows = 0;  // Along Y

    // ---- Build / update ----

    void BuildGridMesh();
    void UpdateMeshSection();
    void RecalculateNormals();

    // Convert world XY → grid column/row index
    bool WorldToGrid(FVector WorldPos, int32& OutCol, int32& OutRow) const;

    // Convert grid col/row → flat vertex index
    int32 GridToIndex(int32 Col, int32 Row) const { return Row * GridCols + Col; }

    // Crater displacement at distance r from centre, given crater params
    // Returns signed Z displacement (negative = depression, positive = rim)
    float CraterProfileZ(float r, float CraterRadius, float CraterDepth, float RimHeight) const;

    // Terrain origin in world space (bottom-left corner of grid)
    FVector GridOrigin() const;
};
