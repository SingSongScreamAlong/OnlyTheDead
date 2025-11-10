// Only The Dead - Medical System
// Copyright 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ActorComponent.h"
#include "../core/GameTypes.h"
#include "MedicalSystem.generated.h"

/**
 * Medical and injury system
 * Realistic WWI medical practices and survival rates
 * Manages wounds, triage, field hospitals, and death
 */

UENUM(BlueprintType)
enum class EInjurySeverity : uint8
{
    Minor       UMETA(DisplayName = "Minor (scratches, bruises)"),
    Moderate    UMETA(DisplayName = "Moderate (shrapnel, concussion)"),
    Severe      UMETA(DisplayName = "Severe (gunshot, amputation needed)"),
    Critical    UMETA(DisplayName = "Critical (near death)"),
    Fatal       UMETA(DisplayName = "Fatal (instant death)")
};

UENUM(BlueprintType)
enum class EBodyPart : uint8
{
    Head        UMETA(DisplayName = "Head"),
    Torso       UMETA(DisplayName = "Torso/Chest"),
    Abdomen     UMETA(DisplayName = "Abdomen"),
    LeftArm     UMETA(DisplayName = "Left Arm"),
    RightArm    UMETA(DisplayName = "Right Arm"),
    LeftLeg     UMETA(DisplayName = "Left Leg"),
    RightLeg    UMETA(DisplayName = "Right Leg")
};

USTRUCT(BlueprintType)
struct FInjury
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injury")
    EInjuryType InjuryType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injury")
    EInjurySeverity Severity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injury")
    EBodyPart AffectedBodyPart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injury")
    float TimeAcquired = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injury")
    bool bIsBleeding = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injury")
    float BleedingRate = 0.0f; // Health lost per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injury")
    bool bIsInfected = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injury")
    bool bIsTreated = false;
};

USTRUCT(BlueprintType)
struct FMedicalSupply
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Medical")
    FString SupplyID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Medical")
    FText SupplyName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Medical")
    int32 Quantity = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Medical")
    TArray<EInjuryType> CanTreat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Medical")
    float TreatmentEffectiveness = 0.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONLYTHEDEAD_API UMedicalSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UMedicalSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========================================================================
    // HEALTH SYSTEM
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    TArray<FInjury> ActiveInjuries;

    /** Apply damage to player */
    UFUNCTION(BlueprintCallable, Category = "Health")
    void ApplyDamage(float DamageAmount, EInjuryType InjuryType, EBodyPart BodyPart);

    /** Heal player */
    UFUNCTION(BlueprintCallable, Category = "Health")
    void Heal(float HealAmount);

    /** Check if player is alive */
    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsAlive() const { return CurrentHealth > 0.0f; }

    /** Get health percentage */
    UFUNCTION(BlueprintPure, Category = "Health")
    float GetHealthPercentage() const { return CurrentHealth / MaxHealth; }

    // ========================================================================
    // INJURY SYSTEM
    // ========================================================================

    /** Add injury to player */
    UFUNCTION(BlueprintCallable, Category = "Injury")
    void AddInjury(FInjury NewInjury);

    /** Treat injury */
    UFUNCTION(BlueprintCallable, Category = "Injury")
    bool TreatInjury(int32 InjuryIndex, const FString& MedicalSupplyID);

    /** Get all active injuries */
    UFUNCTION(BlueprintPure, Category = "Injury")
    TArray<FInjury> GetActiveInjuries() const { return ActiveInjuries; }

    /** Check if has specific injury type */
    UFUNCTION(BlueprintPure, Category = "Injury")
    bool HasInjuryType(EInjuryType InjuryType) const;

    /** Get most severe injury */
    UFUNCTION(BlueprintPure, Category = "Injury")
    FInjury GetMostSevereInjury() const;

    /** Check if any injury is bleeding */
    UFUNCTION(BlueprintPure, Category = "Injury")
    bool IsBleedingOut() const;

    /** Get total bleeding rate */
    UFUNCTION(BlueprintPure, Category = "Injury")
    float GetTotalBleedingRate() const;

    // ========================================================================
    // MEDICAL TREATMENT
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Medical")
    TArray<FMedicalSupply> MedicalSupplies;

    /** Use medical supply */
    UFUNCTION(BlueprintCallable, Category = "Medical")
    bool UseMedicalSupply(const FString& SupplyID, int32 InjuryIndex);

    /** Apply field dressing (stops bleeding) */
    UFUNCTION(BlueprintCallable, Category = "Medical")
    bool ApplyFieldDressing(int32 InjuryIndex);

    /** Inject morphine (pain management) */
    UFUNCTION(BlueprintCallable, Category = "Medical")
    void InjectMorphine();

    /** Apply tourniquet (emergency bleeding control) */
    UFUNCTION(BlueprintCallable, Category = "Medical")
    bool ApplyTourniquet(EBodyPart Limb);

    /** Self-triage (assess own injuries) */
    UFUNCTION(BlueprintCallable, Category = "Medical")
    FText SelfTriage();

    // ========================================================================
    // FIELD HOSPITAL
    // ========================================================================

    /** Transfer to field hospital */
    UFUNCTION(BlueprintCallable, Category = "Medical|Hospital")
    void TransferToFieldHospital();

    /** Receive medical treatment at hospital */
    UFUNCTION(BlueprintCallable, Category = "Medical|Hospital")
    void ReceiveHospitalTreatment(float HoursInCare);

    /** Calculate survival chance based on injuries */
    UFUNCTION(BlueprintPure, Category = "Medical|Hospital")
    float CalculateSurvivalChance() const;

    /** Evacuate to rear hospital */
    UFUNCTION(BlueprintCallable, Category = "Medical|Hospital")
    void EvacuateToRearHospital();

    // ========================================================================
    // INFECTION & DISEASE
    // ========================================================================

    /** Check for infection in untreated wounds */
    UFUNCTION(BlueprintCallable, Category = "Medical|Disease")
    void CheckForInfection(float DeltaTime);

    /** Contract disease */
    UFUNCTION(BlueprintCallable, Category = "Medical|Disease")
    void ContractDisease(EInjuryType DiseaseType);

    /** Treat disease */
    UFUNCTION(BlueprintCallable, Category = "Medical|Disease")
    bool TreatDisease(EInjuryType DiseaseType);

    // ========================================================================
    // DEATH SYSTEM
    // ========================================================================

    /** Check if player should die from injuries */
    UFUNCTION(BlueprintPure, Category = "Medical|Death")
    bool ShouldDieFromInjuries() const;

    /** Calculate time to death from bleeding */
    UFUNCTION(BlueprintPure, Category = "Medical|Death")
    float CalculateTimeToDeathFromBleeding() const;

    /** Trigger death */
    UFUNCTION(BlueprintCallable, Category = "Medical|Death")
    void TriggerDeath(const FString& CauseOfDeath);

    // ========================================================================
    // COMRADE TREATMENT
    // ========================================================================

    /** Treat wounded comrade */
    UFUNCTION(BlueprintCallable, Category = "Medical|Comrades")
    bool TreatWoundedComrade(AActor* ComradeActor);

    /** Carry wounded comrade */
    UFUNCTION(BlueprintCallable, Category = "Medical|Comrades")
    bool CarryWoundedComrade(AActor* ComradeActor);

    /** Call for medic */
    UFUNCTION(BlueprintCallable, Category = "Medical|Comrades")
    void CallForMedic();

protected:
    // ========================================================================
    // INTERNAL STATE
    // ========================================================================

    bool bIsMorphineActive = false;
    float MorphineTimeRemaining = 0.0f;

    void UpdateBleedingDamage(float DeltaTime);
    void UpdateInfections(float DeltaTime);
    void ApplyPainEffects();

    // Historical medical effectiveness (WWI field medicine was limited)
    float GetTreatmentSuccessRate(EInjurySeverity Severity, bool bIsFieldTreatment) const;
};
