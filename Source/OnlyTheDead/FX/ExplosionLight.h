#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExplosionLight.generated.h"

class UPointLightComponent;

// ---------------------------------------------------------------------------
// AExplosionLight
//
// Spawned at each shell impact point to give Lumen correct dynamic GI from
// the explosion. Two light phases:
//
//   Flash (0.0 - 0.3s):     Enormous intensity (500,000 lux, white-orange).
//                            Lumen captures this and illuminates surrounding
//                            smoke columns, trench walls, and churned terrain
//                            with the correct flash colour. Night bombardments
//                            look exactly like historical photographs — the
//                            inside of smoke lit from below by the flash.
//
//   Residual (0.3 - 4.0s):  Burning debris, hot earth, burning propellant gas.
//                            Dimmer orange-red glow that fades over 3-4 seconds.
//                            Gives the impression of smouldering aftermath.
//
// Auto-destroys after residual fades. No manual cleanup needed.
//
// Lumen notes:
//   - Ensure "Lumen Global Illumination" is enabled in Project Settings
//   - Set "Emissive Mesh Light" on any Niagara emitters to feed Lumen further
//   - Radius is intentionally large (1000-3000cm) so Lumen traces pick it up
//   - For indoor trench sections, Lumen's screen-space fallback handles flash
//     bouncing off timber walls even without full GI trace
// ---------------------------------------------------------------------------
UCLASS()
class ONLYTHEDEAD_API AExplosionLight : public AActor
{
    GENERATED_BODY()

public:
    AExplosionLight();

    // ---- Tunable parameters (set per shell type in SpawnExplosionLight()) ----

    // Flash phase: duration (s) and peak intensity (lux)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light|Flash")
    float FlashDuration = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light|Flash")
    float FlashIntensityLux = 500000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light|Flash")
    float FlashRadius = 3000.0f;        // 30m — large for Lumen trace pickup

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light|Flash")
    FLinearColor FlashColor = FLinearColor(1.0f, 0.85f, 0.5f);  // White-orange

    // Residual phase: duration and peak intensity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light|Residual")
    float ResidualDuration = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light|Residual")
    float ResidualIntensityLux = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light|Residual")
    float ResidualRadius = 800.0f;      // Tighter — only nearby surfaces lit

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light|Residual")
    FLinearColor ResidualColor = FLinearColor(1.0f, 0.4f, 0.1f);  // Hot orange-red

    // ---- Static factory (spawns and auto-configures by shell weight) ----

    // Call this from ArtilleryShell::SpawnExplosionFX()
    // ShellWeightKg scales intensity: heavier shells make bigger flashes
    static AExplosionLight* SpawnExplosionLight(UWorld* World, FVector Location, float ShellWeightKg);

    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UPointLightComponent* FlashLight;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UPointLightComponent* ResidualLight;

    float ElapsedTime = 0.0f;
    float TotalDuration = 0.0f;

    void UpdateLightIntensity(float DeltaTime);
};
