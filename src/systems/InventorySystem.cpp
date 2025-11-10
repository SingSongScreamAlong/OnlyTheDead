// Only The Dead - Inventory System Implementation
// Copyright 2025. All Rights Reserved.

#include "InventorySystem.h"
#include "SurvivalSystem.h"
#include "Kismet/GameplayStatics.h"

UInventorySystem::UInventorySystem()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Historical: French soldier carried ~25kg of equipment
    // Rifle (4kg) + Ammo (2kg) + Rations (1kg) + Water (2kg) + Uniform (8kg) + Pack (8kg)
    MaxCarryWeightKG = 25.0f;

    EquippedPrimaryWeapon = EWeaponType::None;
    EquippedSecondaryWeapon = EWeaponType::None;
    PrimaryAmmoCount = 0;
    GrenadeCount = 0;
}

void UInventorySystem::BeginPlay()
{
    Super::BeginPlay();

    InitializeStartingLoadout();

    UE_LOG(LogTemp, Log, TEXT("InventorySystem: Initialized. Max carry weight: %.1fkg"), MaxCarryWeightKG);
}

// ========================================================================
// INVENTORY
// ========================================================================

bool UInventorySystem::AddItem(FInventoryItem Item, int32 Quantity)
{
    if (Quantity <= 0)
    {
        return false;
    }

    // Check weight capacity
    float TotalWeight = Item.WeightKG * Quantity;
    if (!CanCarryWeight(TotalWeight))
    {
        UE_LOG(LogTemp, Warning, TEXT("InventorySystem: Cannot add %s - exceeds carry weight (%.1f/%.1fkg)"),
               *Item.ItemName.ToString(),
               GetCurrentWeight() + TotalWeight,
               MaxCarryWeightKG);
        return false;
    }

    // Check if item already exists (for stackable items)
    bool bItemExists = false;
    for (FInventoryItem& ExistingItem : InventoryItems)
    {
        if (ExistingItem.ItemID == Item.ItemID)
        {
            // Stack if possible
            int32 AvailableStack = ExistingItem.MaxStackSize - ExistingItem.Quantity;
            int32 QuantityToAdd = FMath::Min(Quantity, AvailableStack);

            ExistingItem.Quantity += QuantityToAdd;
            Quantity -= QuantityToAdd;

            bItemExists = true;

            UE_LOG(LogTemp, Log, TEXT("InventorySystem: Stacked %s (now x%d)"),
                   *Item.ItemName.ToString(),
                   ExistingItem.Quantity);

            if (Quantity <= 0)
            {
                return true;
            }
        }
    }

    // Add new stack(s) if needed
    while (Quantity > 0)
    {
        FInventoryItem NewStack = Item;
        NewStack.Quantity = FMath::Min(Quantity, Item.MaxStackSize);
        InventoryItems.Add(NewStack);

        Quantity -= NewStack.Quantity;

        UE_LOG(LogTemp, Log, TEXT("InventorySystem: Added %s x%d (Weight: %.2fkg)"),
               *NewStack.ItemName.ToString(),
               NewStack.Quantity,
               NewStack.WeightKG * NewStack.Quantity);
    }

    return true;
}

bool UInventorySystem::RemoveItem(const FString& ItemID, int32 Quantity)
{
    if (Quantity <= 0)
    {
        return false;
    }

    int32 QuantityToRemove = Quantity;

    for (int32 i = InventoryItems.Num() - 1; i >= 0; i--)
    {
        FInventoryItem& Item = InventoryItems[i];

        if (Item.ItemID == ItemID)
        {
            int32 QuantityFromThisStack = FMath::Min(QuantityToRemove, Item.Quantity);
            Item.Quantity -= QuantityFromThisStack;
            QuantityToRemove -= QuantityFromThisStack;

            if (Item.Quantity <= 0)
            {
                UE_LOG(LogTemp, Log, TEXT("InventorySystem: Removed %s (stack depleted)"),
                       *Item.ItemName.ToString());
                InventoryItems.RemoveAt(i);
            }

            if (QuantityToRemove <= 0)
            {
                return true;
            }
        }
    }

    // Not enough items found
    if (QuantityToRemove > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventorySystem: Could not remove %d of item %s (insufficient quantity)"),
               Quantity,
               *ItemID);
        return false;
    }

    return true;
}

bool UInventorySystem::UseItem(const FString& ItemID)
{
    for (int32 i = 0; i < InventoryItems.Num(); i++)
    {
        FInventoryItem& Item = InventoryItems[i];

        if (Item.ItemID == ItemID)
        {
            if (!Item.bIsConsumable)
            {
                UE_LOG(LogTemp, Warning, TEXT("InventorySystem: %s is not consumable"),
                       *Item.ItemName.ToString());
                return false;
            }

            // Apply item effects
            AActor* Owner = GetOwner();
            if (Owner)
            {
                // Find SurvivalSystem component
                USurvivalSystem* SurvivalSystem = Owner->FindComponentByClass<USurvivalSystem>();
                if (SurvivalSystem)
                {
                    // Apply hunger restoration
                    if (Item.HungerRestoration > 0.0f)
                    {
                        SurvivalSystem->ModifyMeter(ESurvivalMeterType::Hunger, Item.HungerRestoration);
                        UE_LOG(LogTemp, Log, TEXT("InventorySystem: Ate %s (+%.0f hunger)"),
                               *Item.ItemName.ToString(),
                               Item.HungerRestoration);
                    }

                    // Apply thirst restoration
                    if (Item.ThirstRestoration > 0.0f)
                    {
                        SurvivalSystem->ModifyMeter(ESurvivalMeterType::Thirst, Item.ThirstRestoration);
                        UE_LOG(LogTemp, Log, TEXT("InventorySystem: Drank %s (+%.0f thirst)"),
                               *Item.ItemName.ToString(),
                               Item.ThirstRestoration);

                        // Contaminated water causes hygiene/health issues
                        if (Item.bIsContaminated)
                        {
                            SurvivalSystem->ModifyMeter(ESurvivalMeterType::Hygiene, -15.0f);
                            UE_LOG(LogTemp, Warning, TEXT("InventorySystem: Water was contaminated! (-15 hygiene)"));
                        }
                    }
                }
            }

            // Consume one unit
            RemoveItem(ItemID, 1);

            return true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("InventorySystem: Item %s not found in inventory"), *ItemID);
    return false;
}

FInventoryItem UInventorySystem::GetItem(const FString& ItemID) const
{
    for (const FInventoryItem& Item : InventoryItems)
    {
        if (Item.ItemID == ItemID)
        {
            return Item;
        }
    }

    return FInventoryItem();
}

bool UInventorySystem::HasItem(const FString& ItemID, int32 MinQuantity) const
{
    int32 TotalQuantity = 0;

    for (const FInventoryItem& Item : InventoryItems)
    {
        if (Item.ItemID == ItemID)
        {
            TotalQuantity += Item.Quantity;
        }
    }

    return TotalQuantity >= MinQuantity;
}

float UInventorySystem::GetCurrentWeight() const
{
    float TotalWeight = 0.0f;

    for (const FInventoryItem& Item : InventoryItems)
    {
        TotalWeight += Item.WeightKG * Item.Quantity;
    }

    return TotalWeight;
}

bool UInventorySystem::CanCarryWeight(float AdditionalWeightKG) const
{
    return (GetCurrentWeight() + AdditionalWeightKG) <= MaxCarryWeightKG;
}

TArray<FInventoryItem> UInventorySystem::GetItemsByCategory(EItemCategory Category) const
{
    TArray<FInventoryItem> FilteredItems;

    for (const FInventoryItem& Item : InventoryItems)
    {
        if (Item.Category == Category)
        {
            FilteredItems.Add(Item);
        }
    }

    return FilteredItems;
}

// ========================================================================
// EQUIPMENT LOADOUT
// ========================================================================

bool UInventorySystem::EquipWeapon(EWeaponType WeaponType, bool bIsPrimary)
{
    // Check if weapon is available in inventory (simplified - would check actual weapon items)

    if (bIsPrimary)
    {
        EquippedPrimaryWeapon = WeaponType;
        UE_LOG(LogTemp, Log, TEXT("InventorySystem: Equipped primary weapon: %d"), (int32)WeaponType);
    }
    else
    {
        EquippedSecondaryWeapon = WeaponType;
        UE_LOG(LogTemp, Log, TEXT("InventorySystem: Equipped secondary weapon: %d"), (int32)WeaponType);
    }

    return true;
}

bool UInventorySystem::ReloadWeapon()
{
    // Check for ammo in inventory
    TArray<FInventoryItem> AmmoItems = GetItemsByCategory(EItemCategory::Ammunition);

    if (AmmoItems.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventorySystem: No ammunition available"));
        return false;
    }

    // Historical: Lebel rifle used 8-round clips
    int32 ClipSize = 8;

    if (EquippedPrimaryWeapon == EWeaponType::Lebel1886)
    {
        ClipSize = 8;
    }
    else if (EquippedPrimaryWeapon == EWeaponType::BerthierRifle)
    {
        ClipSize = 3; // 3-round clip
    }

    // Find compatible ammo
    for (const FInventoryItem& AmmoItem : AmmoItems)
    {
        if (AmmoItem.Quantity >= ClipSize)
        {
            RemoveItem(AmmoItem.ItemID, ClipSize);
            PrimaryAmmoCount += ClipSize;

            UE_LOG(LogTemp, Log, TEXT("InventorySystem: Reloaded weapon (%d rounds)"), ClipSize);
            return true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("InventorySystem: Insufficient ammo for reload"));
    return false;
}

int32 UInventorySystem::GetAmmoCount() const
{
    return PrimaryAmmoCount;
}

// ========================================================================
// RATION SYSTEM
// ========================================================================

int32 UInventorySystem::GetFoodRationCount() const
{
    TArray<FInventoryItem> FoodItems = GetItemsByCategory(EItemCategory::Food);
    int32 TotalRations = 0;

    for (const FInventoryItem& Item : FoodItems)
    {
        TotalRations += Item.Quantity;
    }

    return TotalRations;
}

float UInventorySystem::GetWaterSupplyML() const
{
    TArray<FInventoryItem> WaterItems = GetItemsByCategory(EItemCategory::Water);
    float TotalWaterML = 0.0f;

    for (const FInventoryItem& Item : WaterItems)
    {
        // Assume each water item = 1000ml (1 liter)
        TotalWaterML += Item.Quantity * 1000.0f;
    }

    return TotalWaterML;
}

bool UInventorySystem::ConsumeDailyRation()
{
    // Historical: French daily ration at Verdun
    // - 700g bread
    // - 500g fresh/frozen meat OR 300g canned meat
    // - Coffee, wine, vegetables
    // Simplified: Consume one "food_ration" item

    if (HasItem(TEXT("food_bread_ration_700g"), 1))
    {
        UseItem(TEXT("food_bread_ration_700g"));
        UE_LOG(LogTemp, Log, TEXT("InventorySystem: Consumed daily ration (bread)"));
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("InventorySystem: No rations available!"));
    return false;
}

bool UInventorySystem::RefillCanteen(bool bFromContaminatedSource)
{
    // Historical: French soldiers carried 1-2 liter canteens
    // Water sources often contaminated at Verdun (shell craters, corpses)

    FInventoryItem CanteenWater;
    CanteenWater.ItemID = bFromContaminatedSource ? TEXT("water_contaminated") : TEXT("water_clean");
    CanteenWater.ItemName = bFromContaminatedSource ?
        FText::FromString(TEXT("Contaminated Water (1L)")) :
        FText::FromString(TEXT("Clean Water (1L)"));
    CanteenWater.ItemDescription = FText::FromString(TEXT("Water from canteen"));
    CanteenWater.Category = EItemCategory::Water;
    CanteenWater.WeightKG = 1.0f;
    CanteenWater.Quantity = 1;
    CanteenWater.MaxStackSize = 3; // Max 3 liters carried
    CanteenWater.bIsConsumable = true;
    CanteenWater.ThirstRestoration = 50.0f;
    CanteenWater.bIsContaminated = bFromContaminatedSource;

    if (AddItem(CanteenWater, 1))
    {
        if (bFromContaminatedSource)
        {
            UE_LOG(LogTemp, Warning, TEXT("InventorySystem: Refilled canteen from contaminated source (RISKY!)"));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("InventorySystem: Refilled canteen with clean water"));
        }
        return true;
    }

    return false;
}

// ========================================================================
// INTERNAL FUNCTIONS
// ========================================================================

void UInventorySystem::InitializeStartingLoadout()
{
    // HISTORICAL: Standard French infantry loadout at Verdun, 1916

    // 1. PRIMARY WEAPON: Lebel Model 1886 Rifle (4.0kg)
    EquipWeapon(EWeaponType::Lebel1886, true);

    // 2. AMMUNITION: 120 rounds (8mm Lebel) - 2.0kg
    FInventoryItem Ammo;
    Ammo.ItemID = TEXT("ammo_8mm_lebel");
    Ammo.ItemName = FText::FromString(TEXT("8mm Lebel Cartridges"));
    Ammo.Category = EItemCategory::Ammunition;
    Ammo.WeightKG = 0.017f; // Per round
    Ammo.Quantity = 120;
    Ammo.MaxStackSize = 120;
    AddItem(Ammo, 120);

    PrimaryAmmoCount = 8; // Loaded in rifle

    // 3. GRENADES: 2x F1 grenades - 1.2kg
    FInventoryItem Grenades;
    Grenades.ItemID = TEXT("grenade_f1");
    Grenades.ItemName = FText::FromString(TEXT("F1 Hand Grenade"));
    Grenades.Category = EItemCategory::Weapon;
    Grenades.WeightKG = 0.6f;
    Grenades.Quantity = 2;
    Grenades.MaxStackSize = 4;
    AddItem(Grenades, 2);
    GrenadeCount = 2;

    // 4. RATIONS: 3 days of bread (historical: resupply every 3-4 days)
    FInventoryItem Bread;
    Bread.ItemID = TEXT("food_bread_ration_700g");
    Bread.ItemName = FText::FromString(TEXT("Bread Ration (700g)"));
    Bread.Category = EItemCategory::Food;
    Bread.WeightKG = 0.7f;
    Bread.Quantity = 3;
    Bread.MaxStackSize = 5;
    Bread.bIsConsumable = true;
    Bread.HungerRestoration = 40.0f;
    AddItem(Bread, 3);

    // 5. WATER: 2 liters in canteen
    RefillCanteen(false); // Clean water
    RefillCanteen(false);

    // 6. MEDICAL: Basic field dressing
    FInventoryItem FieldDressing;
    FieldDressing.ItemID = TEXT("medical_field_dressing");
    FieldDressing.ItemName = FText::FromString(TEXT("Field Dressing"));
    FieldDressing.Category = EItemCategory::Medical;
    FieldDressing.WeightKG = 0.1f;
    FieldDressing.Quantity = 2;
    FieldDressing.MaxStackSize = 5;
    FieldDressing.bIsConsumable = true;
    FieldDressing.HealthRestoration = 25.0f;
    AddItem(FieldDressing, 2);

    // 7. PERSONAL: Letters from home (quest item, no weight)
    FInventoryItem Letters;
    Letters.ItemID = TEXT("personal_letters_home");
    Letters.ItemName = FText::FromString(TEXT("Letters from Home"));
    Letters.Category = EItemCategory::Personal;
    Letters.WeightKG = 0.0f;
    Letters.Quantity = 3;
    Letters.MaxStackSize = 10;
    Letters.bIsQuestItem = true;
    AddItem(Letters, 3);

    UE_LOG(LogTemp, Log, TEXT("InventorySystem: Starting loadout initialized. Total weight: %.1fkg/%.1fkg"),
           GetCurrentWeight(),
           MaxCarryWeightKG);
}

float UInventorySystem::CalculateWeightPenalty() const
{
    // Calculate stamina penalty from carrying weight
    // Over 80% capacity = significant penalty

    float WeightRatio = GetCurrentWeight() / MaxCarryWeightKG;

    if (WeightRatio <= 0.8f)
    {
        return 1.0f; // No penalty
    }

    // Linear penalty from 80% to 100% (1.0 to 2.0x stamina depletion)
    float ExcessRatio = (WeightRatio - 0.8f) / 0.2f;
    float Penalty = 1.0f + ExcessRatio;

    return Penalty;
}
