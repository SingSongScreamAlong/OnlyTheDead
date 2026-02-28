#include "Player/ConcussionComponent.h"

#include "Camera/CameraShakeBase.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

UConcussionComponent::UConcussionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f;  // 20 Hz is sufficient for decay
}

void UConcussionComponent::BeginPlay()
{
    Super::BeginPlay();

    // Create the ear ring audio component (plays on demand)
    if (EarRingSound)
    {
        EarRingComp = NewObject<UAudioComponent>(this, TEXT("EarRingAudio"));
        EarRingComp->RegisterComponent();
        EarRingComp->AttachToComponent(
            GetOwner()->GetRootComponent(),
            FAttachmentTransformRules::KeepRelativeTransform
        );
        EarRingComp->SetSound(EarRingSound);
        EarRingComp->bAutoActivate = false;
    }
}

void UConcussionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceLastImpact += DeltaTime;

    // Drain shell shock slowly when not being actively targeted
    // Drain is paused for 5 seconds after any impact (adrenaline keeps it high)
    if (TimeSinceLastImpact > 5.0f && ShellShockLevel > 0.0f)
    {
        ShellShockLevel = FMath::Max(0.0f, ShellShockLevel - ShockDrainPerSecond * DeltaTime);
        UpdateShellShockState();
    }

    // Update movement penalty based on shock level
    // Catatonic: cannot move. ShellShocked: 30% speed. Distressed: 70%. Shaken: 90%.
    switch (CurrentState)
    {
    case EConcussionState::Catatonic:    MovementSpeedMultiplier = 0.0f; break;
    case EConcussionState::ShellShocked: MovementSpeedMultiplier = 0.30f; break;
    case EConcussionState::Distressed:   MovementSpeedMultiplier = 0.70f; break;
    case EConcussionState::Shaken:       MovementSpeedMultiplier = 0.90f; break;
    default:                             MovementSpeedMultiplier = 1.0f;  break;
    }

    // Audio muffling — active at Distressed or higher
    const bool bShouldMuffle = (CurrentState >= EConcussionState::Distressed);
    if (bShouldMuffle != bAudioMuffled)
    {
        bAudioMuffled = bShouldMuffle;
        // Blueprint reads bAudioMuffled and pushes/pops a DSP muffling mix
    }
}

// ---------------------------------------------------------------------------
// Main notification — called by ArtilleryShell::Detonate
// ---------------------------------------------------------------------------

void UConcussionComponent::NotifyExplosion(
    FVector ImpactLocation, float BlastRadius, float ShellWeightKg)
{
    if (!GetOwner()) return;

    const float Distance = FVector::Dist(GetOwner()->GetActorLocation(), ImpactLocation);

    TimeSinceLastImpact = 0.0f;

    if (Distance <= NearMissRadius)
    {
        ApplyNearMiss(ShellWeightKg);
    }
    else if (Distance <= CloseRadius)
    {
        // Scale effect by how close within the close zone
        const float T = 1.0f - (Distance - NearMissRadius) / (CloseRadius - NearMissRadius);
        ApplyClose(ShellWeightKg * FMath::Lerp(0.5f, 1.0f, T));
    }
    else if (Distance <= MediumRadius)
    {
        ApplyMedium(ShellWeightKg);
    }
    else if (Distance <= DistantRadius)
    {
        ApplyDistant();
    }
}

// ---------------------------------------------------------------------------
// Effect tiers
// ---------------------------------------------------------------------------

void UConcussionComponent::ApplyNearMiss(float ShellWeightKg)
{
    // Shell shock accumulation — near miss is the primary contributor
    ShellShockLevel = FMath::Clamp(ShellShockLevel + ShockAddNearMiss, 0.0f, 1.0f);
    UpdateShellShockState();

    // Weight-scaled shake: 75mm=1.0, 155mm=~2.4, 210mm=~4.0, 305mm=~7.0
    const float ShakeScale = FMath::Clamp(
        FMath::Pow(ShellWeightKg / 7.7f, 0.5f), 1.0f, 8.0f);
    TriggerCameraShake(ShakeNearMiss, ShakeScale);

    // Severe ear ring: high pitch, long duration
    TriggerEarRing(1.0f, FMath::Clamp(ShellWeightKg / 10.0f, 1.0f, 5.0f));

    // Stumble — cleared after 2 seconds
    bStumbling = true;
    GetWorld()->GetTimerManager().ClearTimer(StumbleTimer);
    GetWorld()->GetTimerManager().SetTimer(
        StumbleTimer, [this]() { bStumbling = false; }, 2.0f, false);

    // Time dilation near-miss flash (weight 305mm → 0.4s of slow-mo)
    // This gives the player a split-second to register they almost died
    const float DilationDuration = FMath::Clamp(ShellWeightKg / 200.0f, 0.0f, 0.4f);
    if (DilationDuration > 0.05f)
    {
        UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.3f);
        FTimerHandle DilationTimer;
        GetWorld()->GetTimerManager().SetTimer(DilationTimer, [this]()
        {
            UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
        }, DilationDuration, false);
    }

    UE_LOG(LogTemp, Verbose,
        TEXT("ConcussionComponent: NEAR MISS — ShellShock=%.2f State=%s"),
        ShellShockLevel,
        *UEnum::GetValueAsString(CurrentState));
}

void UConcussionComponent::ApplyClose(float ShellWeightKg)
{
    ShellShockLevel = FMath::Clamp(ShellShockLevel + ShockAddClose, 0.0f, 1.0f);
    UpdateShellShockState();

    const float ShakeScale = FMath::Clamp(FMath::Pow(ShellWeightKg / 7.7f, 0.4f), 1.0f, 5.0f);
    TriggerCameraShake(ShakeClose, ShakeScale);

    // Moderate ear ring
    TriggerEarRing(0.6f, 1.0f);
}

void UConcussionComponent::ApplyMedium(float ShellWeightKg)
{
    ShellShockLevel = FMath::Clamp(ShellShockLevel + ShockAddMedium, 0.0f, 1.0f);
    UpdateShellShockState();

    TriggerCameraShake(ShakeMedium, 1.0f);

    // Brief muffling only for large shells
    if (ShellWeightKg > 40.0f)
    {
        TriggerEarRing(0.2f, 0.3f);
    }
}

void UConcussionComponent::ApplyDistant()
{
    // Distant impacts: only ground rumble, no shock accumulation
    // But in heavy barrages these accumulate perceptually — constant background
    TriggerCameraShake(ShakeDistant, 0.5f);

    if (GroundRumbleSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, GroundRumbleSound, GetOwner()->GetActorLocation());
    }
}

// ---------------------------------------------------------------------------
// State machine
// ---------------------------------------------------------------------------

EConcussionState UConcussionComponent::ComputeStateFromLevel() const
{
    if (ShellShockLevel >= 1.0f)   return EConcussionState::Catatonic;
    if (ShellShockLevel >= 0.75f)  return EConcussionState::ShellShocked;
    if (ShellShockLevel >= 0.50f)  return EConcussionState::Distressed;
    if (ShellShockLevel >= 0.25f)  return EConcussionState::Shaken;
    return EConcussionState::Normal;
}

void UConcussionComponent::UpdateShellShockState()
{
    const EConcussionState NewState = ComputeStateFromLevel();
    if (NewState != CurrentState)
    {
        const EConcussionState OldState = CurrentState;
        CurrentState = NewState;
        OnShellShockStateChanged.Broadcast(OldState, NewState);

        UE_LOG(LogTemp, Log, TEXT("ConcussionComponent: State %s → %s (level=%.2f)"),
            *UEnum::GetValueAsString(OldState),
            *UEnum::GetValueAsString(NewState),
            ShellShockLevel);
    }
}

void UConcussionComponent::ClearShellShock()
{
    const EConcussionState OldState = CurrentState;
    ShellShockLevel = 0.0f;
    CurrentState    = EConcussionState::Normal;
    bStumbling      = false;
    bAudioMuffled   = false;
    MovementSpeedMultiplier = 1.0f;

    if (OldState != EConcussionState::Normal)
    {
        OnShellShockStateChanged.Broadcast(OldState, EConcussionState::Normal);
    }
}

// ---------------------------------------------------------------------------
// Camera shake
// ---------------------------------------------------------------------------

void UConcussionComponent::TriggerCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale)
{
    if (!ShakeClass) return;

    APlayerController* PC = Cast<APlayerController>(
        Cast<APawn>(GetOwner()) ? Cast<APawn>(GetOwner())->GetController() : nullptr);
    if (!PC) return;

    if (PC->PlayerCameraManager)
    {
        PC->PlayerCameraManager->StartCameraShake(ShakeClass, Scale);
    }
}

// ---------------------------------------------------------------------------
// Ear ringing
// ---------------------------------------------------------------------------

void UConcussionComponent::TriggerEarRing(float Intensity, float DurationScale)
{
    if (!EarRingComp || !EarRingComp->GetSound()) return;

    // Pitch: higher intensity = higher-pitched ring (more severe damage)
    // Real ear ringing from concussion: 6-12 kHz tinnitus
    const float Pitch = FMath::Lerp(0.8f, 1.5f, Intensity);
    EarRingComp->SetPitchMultiplier(Pitch);
    EarRingComp->SetVolumeMultiplier(Intensity);

    if (EarRingComp->IsPlaying())
    {
        // Already ringing — boost it rather than restart
        EarRingComp->AdjustVolume(0.1f, FMath::Max(EarRingComp->VolumeMultiplier, Intensity));
    }
    else
    {
        EarRingComp->Play();
    }

    // Fade out after duration (base 5s * DurationScale)
    const float FadeOutTime = 5.0f * DurationScale;
    EarRingComp->FadeOut(FadeOutTime, 0.0f);
}
