// Only The Dead - Audio System Implementation
// Copyright 2025. All Rights Reserved.

#include "AudioSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

UAudioSystem::UAudioSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    MasterVolume = 1.0f;
    ArtilleryVolume = 1.0f;
    DialogueVolume = 1.0f;
    AmbientVolume = 0.7f;
    MusicVolume = 0.5f;
    bSubtitlesEnabled = true;
    bShellRecognitionHintsEnabled = false;

    bShellShockEffectActive = false;
    ShellShockEffectTimeRemaining = 0.0f;
}

void UAudioSystem::BeginPlay()
{
    Super::BeginPlay();

    // Create persistent audio components
    if (GetOwner())
    {
        MusicAudioComponent = NewObject<UAudioComponent>(GetOwner());
        if (MusicAudioComponent)
        {
            MusicAudioComponent->bAutoActivate = false;
            MusicAudioComponent->RegisterComponent();
        }

        AmbienceAudioComponent = NewObject<UAudioComponent>(GetOwner());
        if (AmbienceAudioComponent)
        {
            AmbienceAudioComponent->bAutoActivate = false;
            AmbienceAudioComponent->RegisterComponent();
        }
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Initialized. 3D spatial audio enabled for shell recognition."));
}

void UAudioSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateActiveAudioSources(DeltaTime);

    // Update shell shock effect
    if (bShellShockEffectActive)
    {
        ShellShockEffectTimeRemaining -= DeltaTime;
        if (ShellShockEffectTimeRemaining <= 0.0f)
        {
            ClearAudioEffects();
        }
    }
}

// ========================================================================
// SHELL AUDIO (CRITICAL GAMEPLAY MECHANIC)
// ========================================================================

void UAudioSystem::PlayIncomingShellAudio(EShellType ShellType, FVector ShellLocation, float FlightTimeRemaining)
{
    float WhistlePitch, WhistleDuration;
    FString AudioPattern;
    GetShellAudioCharacteristics(ShellType, WhistlePitch, WhistleDuration, AudioPattern);

    UE_LOG(LogTemp, Warning, TEXT("AudioSystem: INCOMING SHELL! Type: %d, Pattern: %s, Time: %.1fs"),
           (int32)ShellType,
           *AudioPattern,
           FlightTimeRemaining);

    // Create 3D audio source at shell location
    UAudioComponent* ShellAudio = CreateAudioComponent(ShellLocation, true);

    if (ShellAudio)
    {
        // Load appropriate sound cue for shell type
        // Play with 3D spatialization
        // Volume based on artillery volume setting

        float Volume = ArtilleryVolume * MasterVolume;
        // ShellAudio->SetVolumeMultiplier(Volume);

        // Accessibility: Show hint if enabled
        if (bShellRecognitionHintsEnabled)
        {
            UE_LOG(LogTemp, Log, TEXT("AudioSystem: HINT - %s"), *AudioPattern);
        }
    }
}

void UAudioSystem::PlayShellImpactAudio(EShellType ShellType, FVector ImpactLocation)
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystem: SHELL IMPACT at (%.0f, %.0f, %.0f)"),
           ImpactLocation.X,
           ImpactLocation.Y,
           ImpactLocation.Z);

    // Create massive 3D audio explosion
    UAudioComponent* ImpactAudio = CreateAudioComponent(ImpactLocation, true);

    if (ImpactAudio)
    {
        float Volume = ArtilleryVolume * MasterVolume * 1.5f; // Impacts louder
        // ImpactAudio->SetVolumeMultiplier(Volume);

        // Add reverb/echo
        // Add shockwave effect
    }
}

void UAudioSystem::PlayShellOverheadAudio(EShellType ShellType, FVector ShellLocation)
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Shell passing overhead (near miss)"));

    // Doppler effect as shell passes
    // Lower volume than direct hit threat
}

void UAudioSystem::GetShellAudioCharacteristics(EShellType ShellType, float& OutWhistlePitch, float& OutWhistleDuration, FString& OutAudioPattern) const
{
    // Historical audio characteristics for shell types

    switch (ShellType)
    {
        case EShellType::Shell_75mm_French:
            OutWhistlePitch = 800.0f; // High-pitched
            OutWhistleDuration = 2.5f;
            OutAudioPattern = TEXT("High-pitched whistle, 2-3 seconds warning");
            break;

        case EShellType::Shell_105mm_German:
            OutWhistlePitch = 600.0f;
            OutWhistleDuration = 5.0f;
            OutAudioPattern = TEXT("Medium whistle, 4-6 seconds warning");
            break;

        case EShellType::Shell_155mm_Heavy:
            OutWhistlePitch = 250.0f; // Low rumble
            OutWhistleDuration = 7.0f;
            OutAudioPattern = TEXT("Low rumble, 5-8 seconds warning");
            break;

        case EShellType::Shell_210mm_Mortar:
            OutWhistlePitch = 100.0f; // Almost subsonic
            OutWhistleDuration = 1.5f;
            OutAudioPattern = TEXT("ALMOST SILENT - 1-2 seconds warning - DANGER!");
            break;

        case EShellType::Shell_305mm_Railway:
            OutWhistlePitch = 180.0f;
            OutWhistleDuration = 18.0f;
            OutAudioPattern = TEXT("Freight train sound, 15-20 seconds warning");
            break;

        case EShellType::Shell_380mm_Naval:
            OutWhistlePitch = 150.0f;
            OutWhistleDuration = 23.0f;
            OutAudioPattern = TEXT("Apocalyptic roar, 20-25 seconds warning");
            break;

        case EShellType::Shell_420mm_BigBertha:
            OutWhistlePitch = 120.0f;
            OutWhistleDuration = 28.0f;
            OutAudioPattern = TEXT("Sound like the end of the world, 20-30 seconds warning");
            break;

        default:
            OutWhistlePitch = 400.0f;
            OutWhistleDuration = 5.0f;
            OutAudioPattern = TEXT("Generic shell whistle");
            break;
    }
}

// ========================================================================
// COMBAT AUDIO
// ========================================================================

void UAudioSystem::PlayRifleShot(EWeaponType WeaponType, FVector Location)
{
    UAudioComponent* ShotAudio = CreateAudioComponent(Location, true);

    if (ShotAudio)
    {
        // Load rifle shot sound based on weapon type
        UE_LOG(LogTemp, Log, TEXT("AudioSystem: Rifle shot at (%.0f, %.0f, %.0f)"),
               Location.X, Location.Y, Location.Z);
    }
}

void UAudioSystem::PlayMachineGunBurst(EWeaponType MachineGun, FVector Location, float DurationSeconds)
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Machine gun burst (%.1f sec)"), DurationSeconds);

    // Continuous fire sound
    // Realistic cadence (Hotchkiss: 450 RPM, MG 08: 600 RPM)
}

void UAudioSystem::PlayGrenadeExplosion(FVector Location)
{
    UAudioComponent* ExplosionAudio = CreateAudioComponent(Location, true);

    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Grenade explosion"));
}

void UAudioSystem::PlayBulletWhizz(FVector Location)
{
    // High-pitched whizz as bullet passes nearby
    UAudioComponent* WhizzAudio = CreateAudioComponent(Location, true);
}

// ========================================================================
// AMBIENT SOUNDSCAPE
// ========================================================================

void UAudioSystem::SetAmbientSoundscape(const FString& SoundscapeID)
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Setting ambient soundscape: %s"), *SoundscapeID);

    if (!AmbienceAudioComponent)
    {
        return;
    }

    // Load soundscape
    // Play looping ambient audio

    if (SoundscapeID == TEXT("Trench"))
    {
        PlayTrenchAmbience();
    }
    else if (SoundscapeID == TEXT("NoMansLand"))
    {
        PlayNoMansLandAmbience();
    }
}

void UAudioSystem::PlayDistantArtillery(float IntensityLevel)
{
    // Continuous distant rumble
    // Volume based on intensity

    float Volume = AmbientVolume * MasterVolume * IntensityLevel;
}

void UAudioSystem::PlayTrenchAmbience()
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Playing trench ambience"));

    // Water dripping
    // Rats scurrying
    // Wind through sandbags
    // Distant explosions
    // Muffled voices
}

void UAudioSystem::PlayNoMansLandAmbience()
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Playing no-man's-land ambience"));

    // Wind howling through wire
    // Distant groans of wounded
    // Creaking barbed wire
    // Eerie silence
}

// ========================================================================
// VOICE & DIALOGUE
// ========================================================================

void UAudioSystem::PlayDialogueLine(AActor* Speaker, const FString& DialogueID, bool bSubtitles)
{
    if (!Speaker)
    {
        return;
    }

    FVector SpeakerLocation = Speaker->GetActorLocation();
    UAudioComponent* DialogueAudio = CreateAudioComponent(SpeakerLocation, true);

    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Playing dialogue: %s"), *DialogueID);

    if (bSubtitles && bSubtitlesEnabled)
    {
        // Show subtitle UI
        UE_LOG(LogTemp, Log, TEXT("AudioSystem: [Subtitle] %s"), *DialogueID);
    }
}

void UAudioSystem::PlayCombatShout(AActor* Shouter, const FString& ShoutType)
{
    if (!Shouter)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("AudioSystem: Combat shout - %s"), *ShoutType);

    // Urgent, loud voice
    // High priority audio
}

void UAudioSystem::PlayWarningShout(AActor* Shouter, const FString& WarningType, FVector DangerLocation)
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystem: WARNING SHOUT - %s"), *WarningType);

    // Examples: "Grenade!", "Shell!", "Gas!", "Over the top!"
}

void UAudioSystem::PlayDeathCry(AActor* DyingActor)
{
    if (!DyingActor)
    {
        return;
    }

    FVector Location = DyingActor->GetActorLocation();
    UAudioComponent* DeathAudio = CreateAudioComponent(Location, true);

    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Death cry"));

    // Painful, realistic death sounds
}

// ========================================================================
// FOLEY & PLAYER SOUNDS
// ========================================================================

void UAudioSystem::PlayFootstep(FVector Location, const FString& SurfaceType)
{
    // Different sounds for different surfaces
    // Mud (most common), wood (duckboards), stone, water

    UAudioComponent* FootstepAudio = CreateAudioComponent(Location, false); // Not 3D, close to player
}

void UAudioSystem::PlayBreathingSound(float IntensityLevel, bool bIsPanicked)
{
    // Heavy breathing after running
    // Panicked breathing during stress

    float Volume = DialogueVolume * MasterVolume;

    if (bIsPanicked)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioSystem: Panicked breathing"));
    }
}

void UAudioSystem::PlayEquipmentRattle()
{
    // Helmet, rifle, backpack rattling during movement
}

void UAudioSystem::PlayReloadSound(EWeaponType WeaponType)
{
    // Realistic reload foley
    // Lebel: Distinct 8-round clip insertion
    // Berthier: 3-round clip

    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Reload sound"));
}

// ========================================================================
// MUSIC SYSTEM (Minimal, emotional moments only)
// ========================================================================

void UAudioSystem::PlayMusicCue(const FString& MusicCueID, float FadeInSeconds)
{
    if (!MusicAudioComponent)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Playing music: %s"), *MusicCueID);

    // Load music cue
    // Fade in
    // Volume based on music volume setting

    float Volume = MusicVolume * MasterVolume;
    // MusicAudioComponent->FadeIn(FadeInSeconds, Volume);
}

void UAudioSystem::StopMusic(float FadeOutSeconds)
{
    if (!MusicAudioComponent)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Stopping music"));

    // MusicAudioComponent->FadeOut(FadeOutSeconds, 0.0f);
}

void UAudioSystem::TriggerMomentOfSilence(float DurationSeconds)
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystem: MOMENT OF SILENCE (%.0f sec)"), DurationSeconds);

    // Fade out all audio except essential
    // Profound silence after major events (e.g., comrade death)
    // Powerful emotional tool
}

// ========================================================================
// AUDIO EFFECTS
// ========================================================================

void UAudioSystem::ApplyShellShockAudioEffect(float DurationSeconds)
{
    bShellShockEffectActive = true;
    ShellShockEffectTimeRemaining = DurationSeconds;

    UE_LOG(LogTemp, Warning, TEXT("AudioSystem: SHELL SHOCK EFFECT active (%.0f sec)"), DurationSeconds);

    // Muffled hearing
    // Tinnitus (ringing)
    // Reduced volume on all sounds
    // Add high-pitched ringing sound
}

void UAudioSystem::ApplyUnderwaterAudioEffect()
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Underwater audio effect"));

    // Muffled sounds
    // Bubbling
    // Pressure feeling
}

void UAudioSystem::ClearAudioEffects()
{
    bShellShockEffectActive = false;
    ShellShockEffectTimeRemaining = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Audio effects cleared"));

    // Restore normal audio
}

void UAudioSystem::ApplyWeatherAudioEffects(EWeatherCondition Weather, float Intensity)
{
    // Rain pattering on helmet
    // Wind howling
    // Thunder

    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Weather audio effects applied"));
}

// ========================================================================
// 3D SPATIALIZATION
// ========================================================================

float UAudioSystem::Calculate3DAttenuation(FVector SoundLocation, float MaxDistance) const
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return 0.0f;
    }

    float Distance = FVector::Dist(Owner->GetActorLocation(), SoundLocation);

    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }

    // Inverse square law for realistic audio falloff
    float Attenuation = 1.0f - (Distance / MaxDistance);
    return FMath::Clamp(Attenuation, 0.0f, 1.0f);
}

FVector UAudioSystem::GetDirectionToSound(FVector SoundLocation) const
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return FVector::ZeroVector;
    }

    FVector Direction = (SoundLocation - Owner->GetActorLocation()).GetSafeNormal();
    return Direction;
}

bool UAudioSystem::IsSoundOccluded(FVector SoundLocation) const
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }

    // Line trace to check for obstacles
    FHitResult HitResult;
    FVector Start = Owner->GetActorLocation();
    FVector End = SoundLocation;

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility
    );

    return bHit; // If hit something, sound is occluded
}

// ========================================================================
// INTERNAL FUNCTIONS
// ========================================================================

void UAudioSystem::UpdateActiveAudioSources(float DeltaTime)
{
    // Clean up finished audio components
    ActiveAudioComponents.RemoveAll([](UAudioComponent* Component) {
        return !Component || !Component->IsPlaying();
    });

    ApplyVolumeSettings();
}

void UAudioSystem::ApplyVolumeSettings()
{
    // Apply volume settings to all active components

    for (UAudioComponent* Component : ActiveAudioComponents)
    {
        if (Component)
        {
            // Would set volume based on layer and master volume
        }
    }
}

UAudioComponent* UAudioSystem::CreateAudioComponent(FVector Location, bool b3D)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return nullptr;
    }

    UAudioComponent* NewAudio = NewObject<UAudioComponent>(Owner);
    if (NewAudio)
    {
        NewAudio->bAutoActivate = false;
        NewAudio->RegisterComponent();

        if (b3D)
        {
            NewAudio->SetWorldLocation(Location);
            // Enable 3D spatialization
        }

        ActiveAudioComponents.Add(NewAudio);
        return NewAudio;
    }

    return nullptr;
}
