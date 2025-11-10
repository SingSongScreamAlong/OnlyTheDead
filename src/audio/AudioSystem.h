// Only The Dead - Audio System
// Copyright 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ActorComponent.h"
#include "../core/GameTypes.h"
#include "AudioSystem.generated.h"

/**
 * 3D spatial audio system - CRITICAL for survival gameplay
 * Shell recognition by audio is core game mechanic
 * Implements authentic WWI soundscape
 */

UENUM(BlueprintType)
enum class EAudioLayer : uint8
{
    Ambient         UMETA(DisplayName = "Ambient (wind, distant sounds)"),
    Artillery       UMETA(DisplayName = "Artillery (incoming/impact)"),
    SmallArms       UMETA(DisplayName = "Small Arms Fire"),
    Voices          UMETA(DisplayName = "Voices (dialogue, shouts)"),
    Foley           UMETA(DisplayName = "Foley (footsteps, equipment)"),
    Music           UMETA(DisplayName = "Music (rare, emotional moments)"),
    UI              UMETA(DisplayName = "UI Sounds")
};

USTRUCT(BlueprintType)
struct FAudioCueData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString CueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FText CueName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudioLayer Layer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxAudibleDistanceMeters = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bUse3DSpatialization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIsLooping = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONLYTHEDEAD_API UAudioSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudioSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========================================================================
    // SHELL AUDIO (CRITICAL GAMEPLAY MECHANIC)
    // ========================================================================

    /** Play incoming shell whistle (player must identify) */
    UFUNCTION(BlueprintCallable, Category = "Audio|Artillery")
    void PlayIncomingShellAudio(EShellType ShellType, FVector ShellLocation, float FlightTimeRemaining);

    /** Play shell impact audio */
    UFUNCTION(BlueprintCallable, Category = "Audio|Artillery")
    void PlayShellImpactAudio(EShellType ShellType, FVector ImpactLocation);

    /** Play shell passing overhead audio (near miss) */
    UFUNCTION(BlueprintCallable, Category = "Audio|Artillery")
    void PlayShellOverheadAudio(EShellType ShellType, FVector ShellLocation);

    /** Get shell audio characteristics for identification */
    UFUNCTION(BlueprintPure, Category = "Audio|Artillery")
    void GetShellAudioCharacteristics(EShellType ShellType, float& OutWhistlePitch, float& OutWhistleDuration, FString& OutAudioPattern) const;

    // ========================================================================
    // COMBAT AUDIO
    // ========================================================================

    /** Play rifle shot */
    UFUNCTION(BlueprintCallable, Category = "Audio|Combat")
    void PlayRifleShot(EWeaponType WeaponType, FVector Location);

    /** Play machine gun burst */
    UFUNCTION(BlueprintCallable, Category = "Audio|Combat")
    void PlayMachineGunBurst(EWeaponType MachineGun, FVector Location, float DurationSeconds);

    /** Play grenade explosion */
    UFUNCTION(BlueprintCallable, Category = "Audio|Combat")
    void PlayGrenadeExplosion(FVector Location);

    /** Play bullet impact/whizz */
    UFUNCTION(BlueprintCallable, Category = "Audio|Combat")
    void PlayBulletWhizz(FVector Location);

    // ========================================================================
    // AMBIENT SOUNDSCAPE
    // ========================================================================

    /** Set ambient soundscape for location */
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetAmbientSoundscape(const FString& SoundscapeID);

    /** Play distant artillery (background rumble) */
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void PlayDistantArtillery(float IntensityLevel);

    /** Play trench ambience (water dripping, rats, wind) */
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void PlayTrenchAmbience();

    /** Play no-man's-land ambience (wind, distant groans) */
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void PlayNoMansLandAmbience();

    // ========================================================================
    // VOICE & DIALOGUE
    // ========================================================================

    /** Play NPC dialogue line */
    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void PlayDialogueLine(AActor* Speaker, const FString& DialogueID, bool bSubtitles = true);

    /** Play combat shout */
    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void PlayCombatShout(AActor* Shouter, const FString& ShoutType);

    /** Play warning shout */
    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void PlayWarningShout(AActor* Shouter, const FString& WarningType, FVector DangerLocation);

    /** Play death cry */
    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void PlayDeathCry(AActor* DyingActor);

    // ========================================================================
    // FOLEY & PLAYER SOUNDS
    // ========================================================================

    /** Play footstep */
    UFUNCTION(BlueprintCallable, Category = "Audio|Foley")
    void PlayFootstep(FVector Location, const FString& SurfaceType);

    /** Play breathing (heavy after exertion, panic breathing) */
    UFUNCTION(BlueprintCallable, Category = "Audio|Foley")
    void PlayBreathingSound(float IntensityLevel, bool bIsPanicked);

    /** Play equipment rattle */
    UFUNCTION(BlueprintCallable, Category = "Audio|Foley")
    void PlayEquipmentRattle();

    /** Play reload sound */
    UFUNCTION(BlueprintCallable, Category = "Audio|Foley")
    void PlayReloadSound(EWeaponType WeaponType);

    // ========================================================================
    // MUSIC SYSTEM (Minimal, emotional moments only)
    // ========================================================================

    /** Play music cue (rare) */
    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void PlayMusicCue(const FString& MusicCueID, float FadeInSeconds = 2.0f);

    /** Stop music */
    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void StopMusic(float FadeOutSeconds = 2.0f);

    /** Play moment of silence (after major event) */
    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void TriggerMomentOfSilence(float DurationSeconds);

    // ========================================================================
    // AUDIO EFFECTS
    // ========================================================================

    /** Apply shell shock audio effect (muffled hearing, tinnitus) */
    UFUNCTION(BlueprintCallable, Category = "Audio|Effects")
    void ApplyShellShockAudioEffect(float DurationSeconds);

    /** Apply underwater/drowning audio effect */
    UFUNCTION(BlueprintCallable, Category = "Audio|Effects")
    void ApplyUnderwaterAudioEffect();

    /** Clear audio effects */
    UFUNCTION(BlueprintCallable, Category = "Audio|Effects")
    void ClearAudioEffects();

    /** Apply weather audio effects (rain, wind) */
    UFUNCTION(BlueprintCallable, Category = "Audio|Effects")
    void ApplyWeatherAudioEffects(EWeatherCondition Weather, float Intensity);

    // ========================================================================
    // 3D SPATIALIZATION
    // ========================================================================

    /** Calculate 3D audio attenuation */
    UFUNCTION(BlueprintPure, Category = "Audio|3D")
    float Calculate3DAttenuation(FVector SoundLocation, float MaxDistance) const;

    /** Get direction to sound source (for spatialization) */
    UFUNCTION(BlueprintPure, Category = "Audio|3D")
    FVector GetDirectionToSound(FVector SoundLocation) const;

    /** Check if sound is occluded (behind walls, in trenches) */
    UFUNCTION(BlueprintPure, Category = "Audio|3D")
    bool IsSoundOccluded(FVector SoundLocation) const;

    // ========================================================================
    // AUDIO SETTINGS
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Settings")
    float ArtilleryVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Settings")
    float DialogueVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Settings")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Settings")
    float MusicVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Settings")
    bool bSubtitlesEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Settings")
    bool bShellRecognitionHintsEnabled = false; // For accessibility

protected:
    // ========================================================================
    // INTERNAL STATE
    // ========================================================================

    TArray<UAudioComponent*> ActiveAudioComponents;
    UAudioComponent* MusicAudioComponent;
    UAudioComponent* AmbienceAudioComponent;

    bool bShellShockEffectActive = false;
    float ShellShockEffectTimeRemaining = 0.0f;

    void UpdateActiveAudioSources(float DeltaTime);
    void ApplyVolumeSettings();
    UAudioComponent* CreateAudioComponent(FVector Location, bool b3D);
};
