#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrenchSegment.generated.h"

class UStaticMeshComponent;
class UAudioComponent;

// Progression of trench destruction — drives mesh swap and gameplay cover value
UENUM(BlueprintType)
enum class ETrenchState : uint8
{
    Intact      UMETA(DisplayName = "Intact"),        // Full cover 95%
    Damaged     UMETA(DisplayName = "Damaged"),       // Partial cover 60%
    Collapsed   UMETA(DisplayName = "Collapsed"),     // Minimal cover 20%
    Buried      UMETA(DisplayName = "Buried/Crater")  // No cover, open ground
};

// Delegate so GameMode / HUD can react to trench destruction
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnTrenchStateChanged,
    ATrenchSegment*, Segment,
    ETrenchState, NewState
);

/**
 * ATrenchSegment
 *
 * A single destructible section of WWI trench.
 * Trench structure per design doc: 2m wide, 2m deep, sandbag parapets.
 *
 * Destruction model:
 *   Intact (100 HP) → Damaged (50 HP) → Collapsed (0 HP) → Buried (shell crater)
 *
 * Three static mesh slots:
 *   IntactMesh   — timber-lined trench walls, duckboards, sandbags
 *   DamagedMesh  — cracked timbers, partial wall collapse
 *   CollapsedMesh— rubble pile, minimal recognisable structure
 *
 * Cover value is queried by the player's SurvivalComponent to determine
 * how much morale protection this segment provides when the player shelters here.
 */
UCLASS()
class ONLYTHEDEAD_API ATrenchSegment : public AActor
{
    GENERATED_BODY()

public:
    ATrenchSegment();

    // ---- Meshes (assign in Blueprint child class) ----

    // Displayed when state == Intact
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trench|Mesh")
    UStaticMeshComponent* IntactMesh;

    // Displayed when state == Damaged
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trench|Mesh")
    UStaticMeshComponent* DamagedMesh;

    // Displayed when state == Collapsed or Buried
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trench|Mesh")
    UStaticMeshComponent* CollapsedMesh;

    // ---- Audio ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trench|Audio")
    USoundBase* CollapseSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trench|Audio")
    USoundBase* ImpactSound;

    // ---- Stats ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trench|Stats", meta = (ClampMin = "1"))
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Trench|Stats")
    float CurrentHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Trench|Stats")
    ETrenchState TrenchState = ETrenchState::Intact;

    // Cover value 0–1 (1 = full cover, 0 = open)
    UFUNCTION(BlueprintPure, Category = "Trench")
    float GetCoverValue() const;

    // Called by artillery shell blast radius damage system
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
                              AController* EventInstigator, AActor* DamageCauser) override;

    // Force a specific state (useful for level design / scripting)
    UFUNCTION(BlueprintCallable, Category = "Trench")
    void SetTrenchState(ETrenchState NewState);

    UPROPERTY(BlueprintAssignable, Category = "Trench|Events")
    FOnTrenchStateChanged OnTrenchStateChanged;

    // Whether this segment is adjacent to the one at Index (for chain collapse)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trench|Layout")
    int32 SegmentIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trench|Layout")
    TArray<ATrenchSegment*> AdjacentSegments;

protected:
    virtual void BeginPlay() override;

private:
    void UpdateMeshVisibility();
    void TriggerCollapsePhysics();

    // Propagate damage to neighbours (heavy shells collapse adjacent segments)
    void SpreadDamageToAdjacent(float Fraction);

    UAudioComponent* AudioComp;
};
