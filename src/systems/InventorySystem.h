// Only The Dead - Inventory System
// Copyright 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ActorComponent.h"
#include "../core/GameTypes.h"
#include "InventorySystem.generated.h"

/**
 * Inventory and equipment management system
 * Realistic carrying capacity based on French soldier loadout
 * Weight affects stamina depletion
 *
 * PERSISTENT WORLD SUPPORT:
 * - Physical supply depots at GPS coordinates
 * - Resupply points (field kitchens, ammunition dumps, medical stations)
 * - Player must travel to depots to resupply (can't carry 303 days of supplies)
 * - Depot accessibility changes as battle progresses
 */

UENUM(BlueprintType)
enum class EItemCategory : uint8
{
    Weapon          UMETA(DisplayName = "Weapon"),
    Ammunition      UMETA(DisplayName = "Ammunition"),
    Food            UMETA(DisplayName = "Food"),
    Water           UMETA(DisplayName = "Water"),
    Medical         UMETA(DisplayName = "Medical Supplies"),
    Equipment       UMETA(DisplayName = "Equipment"),
    Personal        UMETA(DisplayName = "Personal Items"),
    Quest           UMETA(DisplayName = "Quest Items")
};

USTRUCT(BlueprintType)
struct FInventoryItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FString ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FText ItemDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    EItemCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    float WeightKG = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 Quantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 MaxStackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    bool bIsConsumable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    bool bIsQuestItem = false;

    // Food/Water specific
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
    float HungerRestoration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
    float ThirstRestoration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
    bool bIsContaminated = false;

    // Medical specific
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Medical")
    float HealthRestoration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Medical")
    TArray<EInjuryType> TreatsInjuryTypes;
};

// ========================================================================
// PERSISTENT WORLD: SUPPLY DEPOTS
// ========================================================================

UENUM(BlueprintType)
enum class ESupplyDepotType : uint8
{
    AmmunitionDump      UMETA(DisplayName = "Ammunition Dump"),
    FieldKitchen        UMETA(DisplayName = "Field Kitchen"),
    MedicalStation      UMETA(DisplayName = "Medical Station"),
    WaterPoint          UMETA(DisplayName = "Water Point"),
    QuartermasterPost   UMETA(DisplayName = "Quartermaster Post (All Supplies)"),
    ForwardPost         UMETA(DisplayName = "Forward Supply Post"),
    RearEchelon         UMETA(DisplayName = "Rear Echelon Supply")
};

UENUM(BlueprintType)
enum class ESupplyDepotStatus : uint8
{
    Active              UMETA(DisplayName = "Active (Available)"),
    LowSupply           UMETA(DisplayName = "Low Supply"),
    Depleted            UMETA(DisplayName = "Depleted"),
    UnderBombardment    UMETA(DisplayName = "Under Bombardment"),
    Captured            UMETA(DisplayName = "Captured by Enemy"),
    Destroyed           UMETA(DisplayName = "Destroyed")
};

USTRUCT(BlueprintType)
struct FSupplyDepot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SupplyDepot")
    FString DepotID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SupplyDepot")
    FText DepotName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SupplyDepot")
    ESupplyDepotType DepotType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SupplyDepot")
    ESupplyDepotStatus Status = ESupplyDepotStatus::Active;

    /** World coordinates in persistent map */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SupplyDepot|Location")
    FVector WorldLocation = FVector::ZeroVector;

    /** Original GPS coordinates (for reference) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SupplyDepot|Location")
    FVector2D GPS_Coordinates = FVector2D::ZeroVector; // (Lat, Lon)

    /** Interaction radius (meters) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SupplyDepot|Location")
    float InteractionRadiusMeters = 50.0f;

    /** Distance to front line (affects supply quality) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SupplyDepot")
    float DistanceToFrontKM = 5.0f;

    /** Supply levels per category (0.0 = empty, 1.0 = fully stocked) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SupplyDepot|Inventory")
    TMap<EItemCategory, float> SupplyLevels;

    /** Last resupply time (for replenishment simulation) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SupplyDepot")
    FDateTime LastResupplyTime;

    /** Historical notes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SupplyDepot")
    FText HistoricalNotes;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONLYTHEDEAD_API UInventorySystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventorySystem();

protected:
    virtual void BeginPlay() override;

public:
    // ========================================================================
    // INVENTORY
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<FInventoryItem> InventoryItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    float MaxCarryWeightKG = 25.0f; // Historical French soldier loadout

    /** Add item to inventory */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(FInventoryItem Item, int32 Quantity = 1);

    /** Remove item from inventory */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(const FString& ItemID, int32 Quantity = 1);

    /** Use/consume item */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UseItem(const FString& ItemID);

    /** Get item by ID */
    UFUNCTION(BlueprintPure, Category = "Inventory")
    FInventoryItem GetItem(const FString& ItemID) const;

    /** Check if inventory contains item */
    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool HasItem(const FString& ItemID, int32 MinQuantity = 1) const;

    /** Get current total weight */
    UFUNCTION(BlueprintPure, Category = "Inventory")
    float GetCurrentWeight() const;

    /** Check if can carry additional weight */
    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool CanCarryWeight(float AdditionalWeightKG) const;

    /** Get items by category */
    UFUNCTION(BlueprintPure, Category = "Inventory")
    TArray<FInventoryItem> GetItemsByCategory(EItemCategory Category) const;

    // ========================================================================
    // EQUIPMENT LOADOUT
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    EWeaponType EquippedPrimaryWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    EWeaponType EquippedSecondaryWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    int32 PrimaryAmmoCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    int32 GrenadeCount = 0;

    /** Equip weapon from inventory */
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    bool EquipWeapon(EWeaponType WeaponType, bool bIsPrimary = true);

    /** Reload equipped weapon */
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    bool ReloadWeapon();

    /** Get ammo count for equipped weapon */
    UFUNCTION(BlueprintPure, Category = "Equipment")
    int32 GetAmmoCount() const;

    // ========================================================================
    // RATION SYSTEM
    // ========================================================================

    /** Get food ration count */
    UFUNCTION(BlueprintPure, Category = "Inventory|Rations")
    int32 GetFoodRationCount() const;

    /** Get water supply (in ML) */
    UFUNCTION(BlueprintPure, Category = "Inventory|Rations")
    float GetWaterSupplyML() const;

    /** Consume daily ration */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Rations")
    bool ConsumeDailyRation();

    /** Refill canteen */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Rations")
    bool RefillCanteen(bool bFromContaminatedSource = false);

    // ========================================================================
    // SUPPLY DEPOTS (PERSISTENT WORLD)
    // ========================================================================

    /** All supply depots in persistent world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SupplyDepots")
    TMap<FString, FSupplyDepot> SupplyDepots;

    /** Initialize all supply depot locations */
    UFUNCTION(BlueprintCallable, Category = "SupplyDepots")
    void InitializeSupplyDepots();

    /** Find nearest supply depot to location */
    UFUNCTION(BlueprintPure, Category = "SupplyDepots")
    FSupplyDepot GetNearestSupplyDepot(FVector PlayerLocation, ESupplyDepotType DepotType = ESupplyDepotType::QuartermasterPost) const;

    /** Check if player is within interaction range of any depot */
    UFUNCTION(BlueprintPure, Category = "SupplyDepots")
    bool IsNearSupplyDepot(FVector PlayerLocation, float& OutDistance, FString& OutDepotID) const;

    /** Interact with supply depot (resupply) */
    UFUNCTION(BlueprintCallable, Category = "SupplyDepots")
    bool InteractWithDepot(const FString& DepotID);

    /** Resupply specific item category from depot */
    UFUNCTION(BlueprintCallable, Category = "SupplyDepots")
    bool ResupplyFromDepot(const FString& DepotID, EItemCategory Category, int32 Amount);

    /** Update depot supply levels (simulates resupply convoys) */
    UFUNCTION(BlueprintCallable, Category = "SupplyDepots")
    void UpdateDepotSupplyLevels(float DeltaTime);

    /** Get depot status description for UI */
    UFUNCTION(BlueprintPure, Category = "SupplyDepots")
    FText GetDepotStatusText(const FString& DepotID) const;

    /** Mark depot as captured/destroyed (historical events) */
    UFUNCTION(BlueprintCallable, Category = "SupplyDepots")
    void SetDepotStatus(const FString& DepotID, ESupplyDepotStatus NewStatus);

protected:
    void InitializeStartingLoadout();
    float CalculateWeightPenalty() const;
};
