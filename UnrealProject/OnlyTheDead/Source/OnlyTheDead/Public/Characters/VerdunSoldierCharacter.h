// Copyright Epic Games, Inc. All Rights Reserved.
// Only The Dead - WWI Survival Game - Battle of Verdun 1916

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VerdunSoldierCharacter.generated.h"

class USurvivalComponent;
class UInventoryComponent;
class UMedicalComponent;

/**
 * French soldier character class for the player
 * Manages survival, combat, injuries, and psychological state
 */
UCLASS()
class ONLYTHEDEAD_API AVerdunSoldierCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AVerdunSoldierCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ===== COMPONENTS =====

	/** Survival system component - manages hunger, thirst, stamina, etc. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USurvivalComponent* SurvivalComponent;

	/** Inventory system component - manages equipment and items */
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	//UInventoryComponent* InventoryComponent;

	/** Medical system component - manages injuries and diseases */
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	//UMedicalComponent* MedicalComponent;

	// ===== CHARACTER IDENTITY =====

	/** Character's French name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Identity")
	FString CharacterName;

	/** Character's rank (Soldat, Caporal, Sergent, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Identity")
	FString Rank;

	/** Regiment assignment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Identity")
	FString Regiment;

	/** Days survived at Verdun */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stats")
	int32 DaysSurvived;

	/** Total rotations completed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stats")
	int32 RotationsCompleted;

	/** Artillery barrages survived */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stats")
	int32 BarragesSurvived;

	// ===== MOVEMENT & STATE =====

	/** Is player currently in cover? */
	UPROPERTY(BlueprintReadOnly, Category = "Character|State")
	bool bIsInCover;

	/** Is player currently prone? */
	UPROPERTY(BlueprintReadOnly, Category = "Character|State")
	bool bIsProne;

	/** Is player currently sprinting? */
	UPROPERTY(BlueprintReadOnly, Category = "Character|State")
	bool bIsSprinting;

	/** Movement speed modifier from survival meters */
	UPROPERTY(BlueprintReadOnly, Category = "Character|State")
	float MovementSpeedModifier;

	// ===== FUNCTIONS =====

	/** Toggle prone state */
	UFUNCTION(BlueprintCallable, Category = "Character|Actions")
	void ToggleProne();

	/** Start/stop sprinting (costs stamina) */
	UFUNCTION(BlueprintCallable, Category = "Character|Actions")
	void SetSprinting(bool bNewSprinting);

	/** Take cover */
	UFUNCTION(BlueprintCallable, Category = "Character|Actions")
	void EnterCover();

	/** Leave cover */
	UFUNCTION(BlueprintCallable, Category = "Character|Actions")
	void LeaveCover();

	/** Get current movement speed including all modifiers */
	UFUNCTION(BlueprintPure, Category = "Character|Movement")
	float GetCurrentMovementSpeed() const;

	/** Update movement speed based on survival meters */
	UFUNCTION(BlueprintCallable, Category = "Character|Movement")
	void UpdateMovementSpeed();

protected:
	/** Default walk speed */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Movement")
	float DefaultWalkSpeed;

	/** Sprint speed multiplier */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Movement")
	float SprintSpeedMultiplier;

	/** Stamina cost per second while sprinting */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Movement")
	float SprintStaminaCostPerSecond;

private:
	/** Handle sprinting stamina drain */
	void HandleSprintingStamina(float DeltaTime);

	/** Calculate movement speed modifiers from survival meters */
	float CalculateMovementSpeedModifier() const;
};
