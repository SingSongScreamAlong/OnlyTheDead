#include "FX/ExplosionLight.h"

#include "Components/PointLightComponent.h"

AExplosionLight::AExplosionLight()
{
    PrimaryActorTick.bCanEverTick = true;

    // Flash light — enormous initial intensity, wide radius for Lumen trace pickup
    FlashLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FlashLight"));
    FlashLight->SetIntensity(0.0f);
    FlashLight->SetAttenuationRadius(FlashRadius);
    FlashLight->SetLightColor(FlashColor);
    FlashLight->SetCastShadows(false);  // Flash is too brief for shadow cost
    FlashLight->bUseInverseSquaredFalloff = true;
    RootComponent = FlashLight;

    // Residual glow — narrower, dimmer, cooler (embers/burning dirt)
    ResidualLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ResidualLight"));
    ResidualLight->SetupAttachment(RootComponent);
    ResidualLight->SetIntensity(0.0f);
    ResidualLight->SetAttenuationRadius(ResidualRadius);
    ResidualLight->SetLightColor(ResidualColor);
    ResidualLight->SetCastShadows(true);   // Residual is slow enough to shadow
    ResidualLight->bUseInverseSquaredFalloff = true;

    // Auto-destroy when done
    InitialLifeSpan = 0.0f;  // We control lifetime manually via Tick
}

void AExplosionLight::BeginPlay()
{
    Super::BeginPlay();
    TotalDuration = FlashDuration + ResidualDuration;
    ElapsedTime   = 0.0f;

    // Set initial intensities
    FlashLight->SetIntensity(FlashIntensityLux);
    FlashLight->SetAttenuationRadius(FlashRadius);
    ResidualLight->SetIntensity(0.0f);
}

void AExplosionLight::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    ElapsedTime += DeltaTime;
    UpdateLightIntensity(DeltaTime);

    if (ElapsedTime >= TotalDuration)
    {
        Destroy();
    }
}

void AExplosionLight::UpdateLightIntensity(float DeltaTime)
{
    if (ElapsedTime <= FlashDuration)
    {
        // Flash phase: exponential decay (fast bright flash)
        // I = I0 * exp(-k*t) where k chosen so it decays to ~1% at FlashDuration
        const float k = 5.0f / FlashDuration;
        const float FlashIntensity = FlashIntensityLux * FMath::Exp(-k * ElapsedTime);
        FlashLight->SetIntensity(FlashIntensity);

        // Transition residual in during second half of flash
        const float ResidualT = FMath::Max(0.0f, (ElapsedTime / FlashDuration) - 0.5f) * 2.0f;
        ResidualLight->SetIntensity(ResidualIntensityLux * ResidualT);
    }
    else
    {
        // Residual phase: FlashLight off, ResidualLight slow linear decay
        FlashLight->SetIntensity(0.0f);

        const float ResidualElapsed = ElapsedTime - FlashDuration;
        const float ResidualT = 1.0f - FMath::Clamp(ResidualElapsed / ResidualDuration, 0.0f, 1.0f);

        // Slightly non-linear fade — ember glow holds then drops
        const float EaseOut = ResidualT * ResidualT;
        ResidualLight->SetIntensity(ResidualIntensityLux * EaseOut);

        // Colour shifts redder as it cools (black-body cooling)
        const FLinearColor CoolColor = FLinearColor::LerpUsingHSV(
            FLinearColor(0.5f, 0.1f, 0.0f),   // dark ember red at t=0
            FLinearColor(0.05f, 0.02f, 0.0f),  // barely visible at t=1
            1.0f - EaseOut
        );
        ResidualLight->SetLightColor(CoolColor);
    }
}

// ---------------------------------------------------------------------------
// Static factory
// ---------------------------------------------------------------------------

AExplosionLight* AExplosionLight::SpawnExplosionLight(
    UWorld* World, FVector Location, float ShellWeightKg)
{
    if (!World) return nullptr;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AExplosionLight* Light = World->SpawnActor<AExplosionLight>(
        AExplosionLight::StaticClass(),
        Location + FVector(0, 0, 50.0f),  // Slightly above ground surface
        FRotator::ZeroRotator,
        Params
    );

    if (!Light) return nullptr;

    // Scale by shell weight:
    //   75mm  (7.7 kg)  → scale 1.0: 500k lux, 30m radius
    //   155mm (43 kg)   → scale 2.4: 1.2M lux, 50m radius
    //   210mm (121 kg)  → scale 4.0: 2.0M lux, 80m radius
    //   305mm (380 kg)  → scale 7.0: 3.5M lux, 140m radius
    const float Scale = FMath::Clamp(FMath::Pow(ShellWeightKg / 7.7f, 0.5f), 1.0f, 8.0f);

    Light->FlashIntensityLux    = 500000.0f * Scale;
    Light->FlashRadius          = 3000.0f   * Scale;
    Light->ResidualIntensityLux = 15000.0f  * Scale;
    Light->ResidualRadius       = 800.0f    * Scale;
    Light->FlashDuration        = FMath::Lerp(0.20f, 0.35f, FMath::Clamp(Scale / 8.0f, 0.0f, 1.0f));
    Light->ResidualDuration     = FMath::Lerp(2.5f, 6.0f,   FMath::Clamp(Scale / 8.0f, 0.0f, 1.0f));
    Light->TotalDuration        = Light->FlashDuration + Light->ResidualDuration;

    return Light;
}
