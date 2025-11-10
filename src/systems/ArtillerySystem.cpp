// Only The Dead - Artillery System Implementation
// Copyright 2025. All Rights Reserved.

#include "ArtillerySystem.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

UArtillerySystem::UArtillerySystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // High precision for shell tracking

    CurrentIntensity = EBombardmentIntensity::None;
    BombardmentTimeRemaining = 0.0f;
    TimeSinceLastShell = 0.0f;
    ShellsInLastMinute = 0;
}

void UArtillerySystem::BeginPlay()
{
    Super::BeginPlay();

    InitializeShellDatabase();
    InitializeAudioSystem();

    UE_LOG(LogTemp, Log, TEXT("ArtillerySystem: Initialized. Ready for bombardment."));
    UE_LOG(LogTemp, Log, TEXT("ArtillerySystem: Historical fact - 70%% of Verdun casualties from artillery"));
}

void UArtillerySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateActivesStrikes(DeltaTime);

    // Update bombardment timer
    if (BombardmentTimeRemaining > 0.0f)
    {
        BombardmentTimeRemaining -= DeltaTime;

        if (BombardmentTimeRemaining <= 0.0f)
        {
            StopBombardment();
        }
    }

    // Schedule next shell based on current intensity
    if (CurrentIntensity != EBombardmentIntensity::None)
    {
        TimeSinceLastShell += DeltaTime;
        ScheduleNextShell();
    }

    // Track shells per minute for shell shock calculation
    ShellTimestamps.RemoveAll([this](float Timestamp) {
        return (UGameplayStatics::GetRealTimeSeconds(GetWorld()) - Timestamp) > 60.0f;
    });
    ShellsInLastMinute = ShellTimestamps.Num();
}

// ========================================================================
// BOMBARDMENT SYSTEM
// ========================================================================

void UArtillerySystem::StartBombardment(EBombardmentIntensity Intensity, float DurationSeconds)
{
    CurrentIntensity = Intensity;
    BombardmentTimeRemaining = DurationSeconds;
    TimeSinceLastShell = 0.0f;

    FString IntensityName;
    switch (Intensity)
    {
        case EBombardmentIntensity::Harassing:
            IntensityName = TEXT("HARASSING FIRE (1-2/min)");
            break;
        case EBombardmentIntensity::Light:
            IntensityName = TEXT("LIGHT BARRAGE (5-10/min)");
            break;
        case EBombardmentIntensity::Heavy:
            IntensityName = TEXT("HEAVY BARRAGE (20-30/min)");
            break;
        case EBombardmentIntensity::Trommelfeuer:
            IntensityName = TEXT("TROMMELFEUER - DRUMFIRE (100+/min)");
            break;
        default:
            IntensityName = TEXT("NONE");
            break;
    }

    UE_LOG(LogTemp, Warning, TEXT("ArtillerySystem: BOMBARDMENT STARTED - %s (Duration: %.1f sec)"),
           *IntensityName,
           DurationSeconds);
}

void UArtillerySystem::StopBombardment()
{
    CurrentIntensity = EBombardmentIntensity::None;
    BombardmentTimeRemaining = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("ArtillerySystem: Bombardment stopped."));
}

void UArtillerySystem::FireShell(EShellType ShellType, FVector TargetLocation, FVector OriginLocation)
{
    if (!ShellDatabase.Contains(ShellType))
    {
        UE_LOG(LogTemp, Error, TEXT("ArtillerySystem: Shell type not found in database"));
        return;
    }

    FShellData ShellData = ShellDatabase[ShellType];

    // Calculate flight time based on ballistics
    // Simplified: Use muzzle velocity and distance
    float Distance = FVector::Dist(OriginLocation, TargetLocation);
    float FlightTime = Distance / (ShellData.MuzzleVelocityMS * 100.0f); // Convert m/s to cm/s

    // Historical: Shells took 15-30 seconds depending on range and caliber
    if (ShellData.CaliberMM >= 300)
    {
        FlightTime = FMath::Clamp(FlightTime, 15.0f, 30.0f);
    }
    else if (ShellData.CaliberMM >= 150)
    {
        FlightTime = FMath::Clamp(FlightTime, 5.0f, 15.0f);
    }
    else
    {
        FlightTime = FMath::Clamp(FlightTime, 2.0f, 8.0f);
    }

    // Create artillery strike
    FArtilleryStrike Strike;
    Strike.ImpactLocation = TargetLocation;
    Strike.ShellType = ShellType;
    Strike.TimeToImpact = FlightTime;
    Strike.bIsIncoming = true;
    Strike.OriginLocation = OriginLocation;
    Strike.FlightTime = FlightTime;

    ActiveStrikes.Add(Strike);

    // Play incoming audio
    PlayIncomingShellAudio(ShellType, Distance);

    // Track shell for statistics
    ShellTimestamps.Add(UGameplayStatics::GetRealTimeSeconds(GetWorld()));

    UE_LOG(LogTemp, Log, TEXT("ArtillerySystem: Shell fired - Type: %dmm, Flight time: %.1fs, Distance: %.0fm"),
           ShellData.CaliberMM,
           FlightTime,
           Distance / 100.0f);
}

float UArtillerySystem::CalculateShellDamage(EShellType ShellType, float DistanceFromImpact) const
{
    if (!ShellDatabase.Contains(ShellType))
    {
        return 0.0f;
    }

    FShellData ShellData = ShellDatabase[ShellType];
    float BlastRadiusCM = ShellData.BlastRadiusMeters * 100.0f;

    // Distance is in cm
    if (DistanceFromImpact > BlastRadiusCM)
    {
        return 0.0f; // Outside blast radius
    }

    // Damage falls off quadratically from center
    float DistanceRatio = DistanceFromImpact / BlastRadiusCM;
    float DamageFalloff = 1.0f - (DistanceRatio * DistanceRatio);

    float FinalDamage = ShellData.Damage * DamageFalloff;

    return FMath::Max(0.0f, FinalDamage);
}

bool UArtillerySystem::IsLocationInDangerZone(FVector Location, float& OutNearestDistance) const
{
    OutNearestDistance = TNumericLimits<float>::Max();
    bool bInDanger = false;

    for (const FArtilleryStrike& Strike : ActiveStrikes)
    {
        if (!Strike.bIsIncoming)
        {
            continue;
        }

        float Distance = FVector::Dist(Location, Strike.ImpactLocation);

        if (!ShellDatabase.Contains(Strike.ShellType))
        {
            continue;
        }

        FShellData ShellData = ShellDatabase[Strike.ShellType];
        float DangerRadiusCM = ShellData.BlastRadiusMeters * 100.0f * 1.5f; // 1.5x blast radius for danger zone

        if (Distance < DangerRadiusCM)
        {
            bInDanger = true;
            OutNearestDistance = FMath::Min(OutNearestDistance, Distance);
        }
    }

    return bInDanger;
}

// ========================================================================
// SHELL RECOGNITION SYSTEM
// ========================================================================

void UArtillerySystem::PlayIncomingShellAudio(EShellType ShellType, float Distance)
{
    if (!ArtilleryAudioComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("ArtillerySystem: No audio component available"));
        return;
    }

    FString AudioCueID = GetShellAudioCueID(ShellType);

    // Load and play appropriate sound cue
    // In production, this would load from Content/Audio/Artillery/
    // Sound would be spatialized 3D audio with HRTF

    UE_LOG(LogTemp, Log, TEXT("ArtillerySystem: Playing shell audio - %s (Distance: %.0fm)"),
           *AudioCueID,
           Distance / 100.0f);

    // Audio should give player recognition cues:
    // - 75mm: High-pitched whistle (2-3 sec warning)
    // - 155mm: Low rumble (5-8 sec warning)
    // - 210mm: Almost silent (1-2 sec warning) - VERY DANGEROUS
    // - 305mm: Freight train roar (15-20 sec warning)
    // - 420mm: Apocalyptic roar (20-30 sec warning)
}

bool UArtillerySystem::AttemptShellIdentification(EShellType GuessedType, EShellType ActualType)
{
    bool bCorrect = (GuessedType == ActualType);

    if (bCorrect)
    {
        UE_LOG(LogTemp, Log, TEXT("ArtillerySystem: CORRECT shell identification! Type: %d"), (int32)ActualType);
        // Award XP, improve survival chances
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ArtillerySystem: INCORRECT shell identification. Guessed: %d, Actual: %d"),
               (int32)GuessedType,
               (int32)ActualType);
        // Penalty to survival, missed opportunity to take optimal cover
    }

    return bCorrect;
}

FString UArtillerySystem::GetShellAudioCueID(EShellType ShellType) const
{
    switch (ShellType)
    {
        case EShellType::Shell_75mm_French:
            return TEXT("SFX_Shell_75mm_Incoming");
        case EShellType::Shell_105mm_German:
            return TEXT("SFX_Shell_105mm_Incoming");
        case EShellType::Shell_155mm_Heavy:
            return TEXT("SFX_Shell_155mm_Incoming");
        case EShellType::Shell_210mm_Mortar:
            return TEXT("SFX_Shell_210mm_Incoming");
        case EShellType::Shell_305mm_Railway:
            return TEXT("SFX_Shell_305mm_Incoming");
        case EShellType::Shell_380mm_Naval:
            return TEXT("SFX_Shell_380mm_Incoming");
        case EShellType::Shell_420mm_BigBertha:
            return TEXT("SFX_Shell_420mm_Incoming");
        default:
            return TEXT("SFX_Shell_Generic");
    }
}

float UArtillerySystem::GetReactionTimeWindow(EShellType ShellType, float FlightTime) const
{
    // Reaction time = Flight time - Audio propagation delay
    // Historical: Soldiers had seconds to react based on shell sound

    if (!ShellDatabase.Contains(ShellType))
    {
        return 0.0f;
    }

    FShellData ShellData = ShellDatabase[ShellType];

    // Larger shells give more warning (louder, longer flight time)
    // But some shells are deceptively quiet (210mm mortars)
    float ReactionWindow = FlightTime;

    // Special case: 210mm mortars are "silent death" - minimal warning
    if (ShellType == EShellType::Shell_210mm_Mortar)
    {
        ReactionWindow *= 0.3f; // Only 30% of flight time is audible warning
    }

    return FMath::Max(0.5f, ReactionWindow); // Minimum 0.5 sec reaction time
}

// ========================================================================
// SHELL DATA
// ========================================================================

void UArtillerySystem::InitializeShellDatabase()
{
    // 75mm French Field Gun (Canon de 75 modèle 1897)
    // HISTORICAL: Most common French artillery piece at Verdun
    FShellData Shell_75mm;
    Shell_75mm.CaliberMM = 75;
    Shell_75mm.BlastRadiusMeters = 8.0f;
    Shell_75mm.Damage = 150.0f;
    Shell_75mm.MuzzleVelocityMS = 529.0f; // Historical: 529 m/s
    Shell_75mm.WhistlePitchHz = 800;
    Shell_75mm.RecognitionHint = TEXT("High-pitched whistle, 2-3 seconds warning");
    ShellDatabase.Add(EShellType::Shell_75mm_French, Shell_75mm);

    // 105mm German Light Howitzer
    FShellData Shell_105mm;
    Shell_105mm.CaliberMM = 105;
    Shell_105mm.BlastRadiusMeters = 12.0f;
    Shell_105mm.Damage = 250.0f;
    Shell_105mm.MuzzleVelocityMS = 395.0f;
    Shell_105mm.WhistlePitchHz = 600;
    Shell_105mm.RecognitionHint = TEXT("Medium whistle, 4-6 seconds warning");
    ShellDatabase.Add(EShellType::Shell_105mm_German, Shell_105mm);

    // 155mm Heavy Artillery
    // HISTORICAL: Workhorse of heavy bombardment at Verdun
    FShellData Shell_155mm;
    Shell_155mm.CaliberMM = 155;
    Shell_155mm.BlastRadiusMeters = 25.0f;
    Shell_155mm.Damage = 400.0f;
    Shell_155mm.MuzzleVelocityMS = 420.0f;
    Shell_155mm.WhistlePitchHz = 250;
    Shell_155mm.RecognitionHint = TEXT("Low rumble, 5-8 seconds warning");
    ShellDatabase.Add(EShellType::Shell_155mm_Heavy, Shell_155mm);

    // 210mm Mortar
    // HISTORICAL: "Silent death" - soldiers feared this more than any other
    FShellData Shell_210mm;
    Shell_210mm.CaliberMM = 210;
    Shell_210mm.BlastRadiusMeters = 30.0f;
    Shell_210mm.Damage = 600.0f;
    Shell_210mm.MuzzleVelocityMS = 220.0f; // Low velocity, high arc
    Shell_210mm.WhistlePitchHz = 100; // Almost subsonic - very quiet
    Shell_210mm.RecognitionHint = TEXT("Almost silent, 1-2 seconds warning - VERY DANGEROUS");
    ShellDatabase.Add(EShellType::Shell_210mm_Mortar, Shell_210mm);

    // 305mm Railway Gun
    FShellData Shell_305mm;
    Shell_305mm.CaliberMM = 305;
    Shell_305mm.BlastRadiusMeters = 50.0f;
    Shell_305mm.Damage = 1000.0f;
    Shell_305mm.MuzzleVelocityMS = 650.0f;
    Shell_305mm.WhistlePitchHz = 180;
    Shell_305mm.RecognitionHint = TEXT("Freight train sound, 15-20 seconds warning");
    ShellDatabase.Add(EShellType::Shell_305mm_Railway, Shell_305mm);

    // 380mm Naval Gun
    FShellData Shell_380mm;
    Shell_380mm.CaliberMM = 380;
    Shell_380mm.BlastRadiusMeters = 60.0f;
    Shell_380mm.Damage = 1500.0f;
    Shell_380mm.MuzzleVelocityMS = 700.0f;
    Shell_380mm.WhistlePitchHz = 150;
    Shell_380mm.RecognitionHint = TEXT("Apocalyptic roar, 20-25 seconds warning");
    ShellDatabase.Add(EShellType::Shell_380mm_Naval, Shell_380mm);

    // 420mm "Big Bertha"
    // HISTORICAL: German superheavy howitzer, psychological terror weapon
    FShellData Shell_420mm;
    Shell_420mm.CaliberMM = 420;
    Shell_420mm.BlastRadiusMeters = 80.0f;
    Shell_420mm.Damage = 2000.0f;
    Shell_420mm.MuzzleVelocityMS = 425.0f;
    Shell_420mm.WhistlePitchHz = 120;
    Shell_420mm.RecognitionHint = TEXT("Sound like the end of the world, 20-30 seconds warning");
    ShellDatabase.Add(EShellType::Shell_420mm_BigBertha, Shell_420mm);

    UE_LOG(LogTemp, Log, TEXT("ArtillerySystem: Shell database initialized with %d shell types"), ShellDatabase.Num());
}

FShellData UArtillerySystem::GetShellData(EShellType ShellType) const
{
    if (ShellDatabase.Contains(ShellType))
    {
        return ShellDatabase[ShellType];
    }

    UE_LOG(LogTemp, Warning, TEXT("ArtillerySystem: Shell type %d not found"), (int32)ShellType);
    return FShellData();
}

// ========================================================================
// CRATER SYSTEM
// ========================================================================

void UArtillerySystem::CreateCrater(FVector Location, EShellType ShellType)
{
    float CraterDepth, CraterDiameter;
    GetCraterDimensions(ShellType, CraterDepth, CraterDiameter);

    UE_LOG(LogTemp, Log, TEXT("ArtillerySystem: Crater created - Diameter: %.1fm, Depth: %.1fm"),
           CraterDiameter,
           CraterDepth);

    // In production, this would:
    // 1. Spawn crater mesh
    // 2. Deform terrain using landscape sculpting
    // 3. Create debris particles
    // 4. Add water puddle if rainy
    // 5. Update navigation mesh
}

void UArtillerySystem::GetCraterDimensions(EShellType ShellType, float& OutDepth, float& OutDiameter) const
{
    if (!ShellDatabase.Contains(ShellType))
    {
        OutDepth = 0.0f;
        OutDiameter = 0.0f;
        return;
    }

    FShellData ShellData = ShellDatabase[ShellType];

    // Historical crater sizes (approximate)
    // Crater diameter ≈ 2-3x blast radius
    // Crater depth ≈ 1/3 to 1/2 diameter

    OutDiameter = ShellData.BlastRadiusMeters * 2.5f;
    OutDepth = OutDiameter * 0.4f;

    // Specific historical data:
    // 75mm: ~2m diameter, 1.5m deep
    // 155mm: ~5m diameter, 3m deep
    // 210mm: ~6m diameter, 4m deep
    // 420mm Big Bertha: ~15m diameter, 10m deep (massive!)
}

// ========================================================================
// MORALE EFFECTS
// ========================================================================

float UArtillerySystem::CalculateMoraleImpact(float DistanceFromPlayer, EShellType ShellType) const
{
    if (!ShellDatabase.Contains(ShellType))
    {
        return 0.0f;
    }

    FShellData ShellData = ShellDatabase[ShellType];
    float BlastRadiusCM = ShellData.BlastRadiusMeters * 100.0f;

    // Morale impact is severe within 3x blast radius
    float MoraleRadius = BlastRadiusCM * 3.0f;

    if (DistanceFromPlayer > MoraleRadius)
    {
        return 0.0f;
    }

    // Calculate morale hit (0-50 points)
    float DistanceRatio = DistanceFromPlayer / MoraleRadius;
    float MoraleHit = (1.0f - DistanceRatio) * 50.0f;

    // Larger shells have greater psychological impact
    float CaliberMultiplier = ShellData.CaliberMM / 155.0f; // Normalized to 155mm
    MoraleHit *= CaliberMultiplier;

    return FMath::Clamp(MoraleHit, 0.0f, 50.0f);
}

bool UArtillerySystem::ShouldTriggerShellShock(float DistanceFromPlayer, int32 StrikesInLastMinute) const
{
    // Shell shock (PTSD) triggered by:
    // 1. Very close calls (< 10m)
    // 2. Prolonged bombardment (20+ shells/min for extended period)
    // 3. Cumulative stress

    bool bVeryCloseCalls = (DistanceFromPlayer < 1000.0f); // 10 meters
    bool bIntenseBombardment = (StrikesInLastMinute > 20);

    // Probability increases with both factors
    float ShockProbability = 0.0f;

    if (bVeryCloseCalls)
    {
        ShockProbability += 0.3f; // 30% chance from close call
    }

    if (bIntenseBombardment)
    {
        ShockProbability += (StrikesInLastMinute / 100.0f); // Up to 100% at Trommelfeuer
    }

    float RandomRoll = FMath::FRand();
    return RandomRoll < ShockProbability;
}

// ========================================================================
// HISTORICAL BOMBARDMENT PATTERNS
// ========================================================================

void UArtillerySystem::StartHistoricalBombardment_Feb21_Opening()
{
    // HISTORICAL: February 21, 1916, 7:15 AM
    // 1,400 German artillery pieces opened fire
    // 9 hours of continuous bombardment
    // Trommelfeuer intensity: 100,000+ shells fired

    UE_LOG(LogTemp, Warning, TEXT("ArtillerySystem: HISTORICAL BOMBARDMENT - Feb 21, 1916, 07:15"));
    UE_LOG(LogTemp, Warning, TEXT("ArtillerySystem: 1,400 guns, 9 hours, 100,000+ shells"));
    UE_LOG(LogTemp, Warning, TEXT("ArtillerySystem: This is Trommelfeuer - DRUMFIRE"));

    // Start Trommelfeuer for 9 hours (historical accuracy)
    float NineHoursInSeconds = 9.0f * 60.0f * 60.0f;
    StartBombardment(EBombardmentIntensity::Trommelfeuer, NineHoursInSeconds);
}

void UArtillerySystem::StartTrommelfeuer(FVector CenterLocation, float RadiusMeters, float DurationMinutes)
{
    // Trommelfeuer = "Drumfire" - continuous rolling barrage
    // Historical: 100+ shells per minute across the front

    UE_LOG(LogTemp, Warning, TEXT("ArtillerySystem: TROMMELFEUER started - Radius: %.0fm, Duration: %.1f min"),
           RadiusMeters,
           DurationMinutes);

    StartBombardment(EBombardmentIntensity::Trommelfeuer, DurationMinutes * 60.0f);

    // In production, this would spawn shells across entire radius
    // Mixed shell types for realistic bombardment pattern
}

EBombardmentIntensity UArtillerySystem::GetHistoricalIntensity(FHistoricalDate Date, FString SectorName) const
{
    // Return historically accurate bombardment intensity for date/sector

    // Feb 21, 1916 - Opening bombardment
    if (Date.Year == 1916 && Date.Month == 2 && Date.Day == 21)
    {
        return EBombardmentIntensity::Trommelfeuer;
    }

    // Fort Douaumont sector (Feb 25-26) - Very heavy
    if (Date.Year == 1916 && Date.Month == 2 && (Date.Day == 25 || Date.Day == 26))
    {
        if (SectorName.Contains(TEXT("Douaumont")))
        {
            return EBombardmentIntensity::Heavy;
        }
    }

    // General frontline sectors - Light to Heavy
    if (SectorName.Contains(TEXT("Front")))
    {
        return EBombardmentIntensity::Light;
    }

    // Rest areas - Harassing fire only
    if (SectorName.Contains(TEXT("Rest")))
    {
        return EBombardmentIntensity::Harassing;
    }

    return EBombardmentIntensity::None;
}

// ========================================================================
// INTERNAL FUNCTIONS
// ========================================================================

void UArtillerySystem::UpdateActivesStrikes(float DeltaTime)
{
    TArray<FArtilleryStrike> StrikesToRemove;

    for (FArtilleryStrike& Strike : ActiveStrikes)
    {
        if (!Strike.bIsIncoming)
        {
            continue;
        }

        Strike.TimeToImpact -= DeltaTime;

        if (Strike.TimeToImpact <= 0.0f)
        {
            // Shell has landed
            SpawnShellImpact(Strike);
            Strike.bIsIncoming = false;
            StrikesToRemove.Add(Strike);
        }
    }

    // Remove impacted strikes
    for (const FArtilleryStrike& Strike : StrikesToRemove)
    {
        ActiveStrikes.Remove(Strike);
    }
}

void UArtillerySystem::SpawnShellImpact(const FArtilleryStrike& Strike)
{
    UE_LOG(LogTemp, Warning, TEXT("ArtillerySystem: SHELL IMPACT at location (%.0f, %.0f, %.0f)"),
           Strike.ImpactLocation.X,
           Strike.ImpactLocation.Y,
           Strike.ImpactLocation.Z);

    // Create crater
    CreateCrater(Strike.ImpactLocation, Strike.ShellType);

    // Spawn explosion VFX
    // Play impact sound
    // Apply damage to nearby actors
    // Apply morale effects
    // Check for player death/injury

    // Visual debug (development only)
    #if WITH_EDITOR
    DrawDebugSphere(
        GetWorld(),
        Strike.ImpactLocation,
        GetShellData(Strike.ShellType).BlastRadiusMeters * 100.0f,
        16,
        FColor::Red,
        false,
        5.0f,
        0,
        10.0f
    );
    #endif
}

void UArtillerySystem::ScheduleNextShell()
{
    // Schedule next shell based on bombardment intensity

    float IntervalBetweenShells = 0.0f;

    switch (CurrentIntensity)
    {
        case EBombardmentIntensity::Harassing:
            IntervalBetweenShells = FMath::RandRange(30.0f, 60.0f); // 1-2 per minute
            break;

        case EBombardmentIntensity::Light:
            IntervalBetweenShells = FMath::RandRange(6.0f, 12.0f); // 5-10 per minute
            break;

        case EBombardmentIntensity::Heavy:
            IntervalBetweenShells = FMath::RandRange(2.0f, 3.0f); // 20-30 per minute
            break;

        case EBombardmentIntensity::Trommelfeuer:
            IntervalBetweenShells = FMath::RandRange(0.3f, 0.6f); // 100+ per minute
            break;

        default:
            return;
    }

    if (TimeSinceLastShell >= IntervalBetweenShells)
    {
        // Fire shell at random location near player
        AActor* Owner = GetOwner();
        if (Owner)
        {
            FVector PlayerLocation = Owner->GetActorLocation();

            // Random offset (100-500 meters)
            float OffsetX = FMath::RandRange(-50000.0f, 50000.0f); // cm
            float OffsetY = FMath::RandRange(-50000.0f, 50000.0f);
            FVector TargetLocation = PlayerLocation + FVector(OffsetX, OffsetY, 0.0f);

            // Random shell type (weighted toward smaller shells)
            int32 ShellTypeRoll = FMath::RandRange(0, 100);
            EShellType ShellType;

            if (ShellTypeRoll < 50)
                ShellType = EShellType::Shell_75mm_French;
            else if (ShellTypeRoll < 75)
                ShellType = EShellType::Shell_105mm_German;
            else if (ShellTypeRoll < 90)
                ShellType = EShellType::Shell_155mm_Heavy;
            else if (ShellTypeRoll < 96)
                ShellType = EShellType::Shell_210mm_Mortar;
            else if (ShellTypeRoll < 99)
                ShellType = EShellType::Shell_305mm_Railway;
            else
                ShellType = EShellType::Shell_420mm_BigBertha;

            FVector OriginLocation = TargetLocation + FVector(FMath::RandRange(-100000.0f, 100000.0f), 0.0f, 0.0f);

            FireShell(ShellType, TargetLocation, OriginLocation);
            TimeSinceLastShell = 0.0f;
        }
    }
}

void UArtillerySystem::InitializeAudioSystem()
{
    // Create audio component for artillery sounds
    if (GetOwner())
    {
        ArtilleryAudioComponent = NewObject<UAudioComponent>(GetOwner());
        if (ArtilleryAudioComponent)
        {
            ArtilleryAudioComponent->bAutoActivate = false;
            ArtilleryAudioComponent->RegisterComponent();

            UE_LOG(LogTemp, Log, TEXT("ArtillerySystem: Audio component initialized"));
        }
    }
}
