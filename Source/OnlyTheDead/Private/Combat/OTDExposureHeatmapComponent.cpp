// Only The Dead - Exposure Heatmap Component Implementation

#include "Combat/OTDExposureHeatmapComponent.h"
#include "GameFramework/Character.h"
#include "Cover/OTDCoverDetectionComponent.h"
#include "Kismet/GameplayStatics.h"

UOTDExposureHeatmapComponent::UOTDExposureHeatmapComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Default settings
	ExposureAccumulationRate = 10.0f; // Builds quickly
	ExposureDecayRate = 5.0f;         // Decays slower (encourages staying in cover)
	ArtilleryAttractionThreshold = 3.0f;
	MovementExposureMultiplier = 1.5f;
	FiringExposureMultiplier = 2.0f;
	VisibilityCheckInterval = 0.5f;

	// Level thresholds
	LowExposureThreshold = 20.0f;
	MediumExposureThreshold = 40.0f;
	HighExposureThreshold = 60.0f;
	CriticalExposureThreshold = 80.0f;

	CurrentExposure = 0.0f;
	CurrentLevel = EOTDExposureLevel::None;
	TimeAtCurrentLevel = 0.0f;
}

void UOTDExposureHeatmapComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentExposure = 0.0f;
	TimeSinceLastVisibilityCheck = 0.0f;
}

void UOTDExposureHeatmapComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateVisibility(DeltaTime);
	UpdateExposure(DeltaTime);
	UpdateExposureLevel();

	TimeAtCurrentLevel += DeltaTime;

	// Check if artillery should be attracted
	if ((CurrentLevel == EOTDExposureLevel::High || CurrentLevel == EOTDExposureLevel::Critical) &&
		TimeAtCurrentLevel >= ArtilleryAttractionThreshold)
	{
		NotifyAIOfHighExposure();
	}
}

// ========== Exposure Query ==========

EOTDExposureLevel UOTDExposureHeatmapComponent::GetExposureLevel() const
{
	if (CurrentExposure >= CriticalExposureThreshold)
	{
		return EOTDExposureLevel::Critical;
	}
	else if (CurrentExposure >= HighExposureThreshold)
	{
		return EOTDExposureLevel::High;
	}
	else if (CurrentExposure >= MediumExposureThreshold)
	{
		return EOTDExposureLevel::Medium;
	}
	else if (CurrentExposure >= LowExposureThreshold)
	{
		return EOTDExposureLevel::Low;
	}
	else
	{
		return EOTDExposureLevel::None;
	}
}

float UOTDExposureHeatmapComponent::GetArtilleryAttractionScore() const
{
	// Artillery is primarily attracted to sustained high exposure
	if (CurrentLevel < EOTDExposureLevel::High)
	{
		return 0.0f;
	}

	// Score increases with exposure level and time
	float BaseScore = CurrentExposure / 100.0f;
	float TimeMultiplier = FMath::Min(TimeAtCurrentLevel / ArtilleryAttractionThreshold, 2.0f);

	return FMath::Clamp(BaseScore * TimeMultiplier, 0.0f, 1.0f);
}

float UOTDExposureHeatmapComponent::GetSniperAttractionScore() const
{
	// Snipers are attracted to medium+ exposure, especially stationary targets
	if (CurrentLevel < EOTDExposureLevel::Medium)
	{
		return 0.0f;
	}

	float BaseScore = CurrentExposure / 100.0f;

	// Snipers prefer stationary targets
	if (CurrentExposureMultiplier < MovementExposureMultiplier)
	{
		BaseScore *= 1.5f; // Bonus for stationary
	}

	return FMath::Clamp(BaseScore, 0.0f, 1.0f);
}

// ========== Manual Control ==========

void UOTDExposureHeatmapComponent::AddExposure(float Amount)
{
	CurrentExposure = FMath::Clamp(CurrentExposure + Amount, 0.0f, 100.0f);
}

void UOTDExposureHeatmapComponent::SetExposure(float Value)
{
	CurrentExposure = FMath::Clamp(Value, 0.0f, 100.0f);
}

void UOTDExposureHeatmapComponent::ResetExposure()
{
	CurrentExposure = 0.0f;
	TimeAtCurrentLevel = 0.0f;
}

// ========== Internal Methods ==========

void UOTDExposureHeatmapComponent::UpdateVisibility(float DeltaTime)
{
	TimeSinceLastVisibilityCheck += DeltaTime;

	if (TimeSinceLastVisibilityCheck < VisibilityCheckInterval)
	{
		return;
	}

	TimeSinceLastVisibilityCheck = 0.0f;

	// Check if visible to enemies
	bWasVisibleLastFrame = IsVisibleToEnemies();

	// Update exposure multiplier based on character state
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{
		// Check if moving
		const FVector Velocity = Character->GetVelocity();
		const bool bIsMoving = Velocity.SizeSquared() > 100.0f; // Moving faster than 10 cm/s

		if (bIsMoving)
		{
			CurrentExposureMultiplier = MovementExposureMultiplier;
		}
		else
		{
			CurrentExposureMultiplier = 1.0f;
		}

		// TODO: Check if firing weapon (would need weapon component)
		// if (bIsFiring) CurrentExposureMultiplier *= FiringExposureMultiplier;
	}
}

void UOTDExposureHeatmapComponent::UpdateExposure(float DeltaTime)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	// Check cover status
	UOTDCoverDetectionComponent* CoverDetection = Character->FindComponentByClass<UOTDCoverDetectionComponent>();
	const bool bIsInCover = CoverDetection && CoverDetection->IsNearCover();
	const float CoverProtection = bIsInCover ? CoverDetection->GetProtectionValue() : 0.0f;

	if (bWasVisibleLastFrame && !bIsInCover)
	{
		// Exposed and visible: accumulate exposure
		const float AccumulationAmount = ExposureAccumulationRate * CurrentExposureMultiplier * DeltaTime;
		CurrentExposure = FMath::Clamp(CurrentExposure + AccumulationAmount, 0.0f, 100.0f);

		TotalExposureTime += DeltaTime;
	}
	else if (bIsInCover)
	{
		// In cover: decay exposure based on cover quality
		const float DecayMultiplier = 1.0f + CoverProtection; // Better cover = faster decay
		const float DecayAmount = ExposureDecayRate * DecayMultiplier * DeltaTime;
		CurrentExposure = FMath::Clamp(CurrentExposure - DecayAmount, 0.0f, 100.0f);
	}
	else if (!bWasVisibleLastFrame)
	{
		// Not visible: slow decay
		const float DecayAmount = ExposureDecayRate * 0.5f * DeltaTime;
		CurrentExposure = FMath::Clamp(CurrentExposure - DecayAmount, 0.0f, 100.0f);
	}
}

void UOTDExposureHeatmapComponent::UpdateExposureLevel()
{
	const EOTDExposureLevel NewLevel = GetExposureLevel();

	if (NewLevel != CurrentLevel)
	{
		const EOTDExposureLevel OldLevel = CurrentLevel;
		CurrentLevel = NewLevel;
		TimeAtCurrentLevel = 0.0f;

		// Broadcast change
		OnExposureLevelChanged.Broadcast(OldLevel, NewLevel);

		// Log level changes
		const TCHAR* LevelNames[] = {TEXT("None"), TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Critical")};
		UE_LOG(LogTemp, Warning, TEXT("OTDExposure: Level changed from %s to %s (value: %.1f)"),
			LevelNames[static_cast<int32>(OldLevel)],
			LevelNames[static_cast<int32>(NewLevel)],
			CurrentExposure);
	}
}

bool UOTDExposureHeatmapComponent::IsVisibleToEnemies() const
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character || !GetWorld())
	{
		return false;
	}

	// Simplified visibility check
	// In full implementation, this would query EQS or AI perception system
	// For now, we check if character is in the open (not in cover)

	UOTDCoverDetectionComponent* CoverDetection = Character->FindComponentByClass<UOTDCoverDetectionComponent>();
	if (!CoverDetection)
	{
		// No cover detection = always assume potentially visible
		return true;
	}

	// If in good cover, not visible
	if (CoverDetection->IsNearCover() && CoverDetection->GetProtectionValue() >= 0.6f)
	{
		return false;
	}

	// Check if there are enemy actors nearby with line of sight
	// This is a simplified check - full implementation would use AI perception
	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), Enemies);

	const FVector CharacterLocation = Character->GetActorLocation();

	for (AActor* Enemy : Enemies)
	{
		if (!Enemy)
		{
			continue;
		}

		const FVector EnemyLocation = Enemy->GetActorLocation();
		const float Distance = FVector::Dist(CharacterLocation, EnemyLocation);

		// Only check enemies within reasonable observation range (500m)
		if (Distance < 50000.0f)
		{
			// Line of sight check
			FHitResult HitResult;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(Character);
			QueryParams.AddIgnoredActor(Enemy);

			if (GetWorld()->LineTraceSingleByChannel(HitResult, EnemyLocation, CharacterLocation, ECC_Visibility, QueryParams))
			{
				if (HitResult.GetActor() == Character)
				{
					// Enemy has line of sight to character
					return true;
				}
			}
			else
			{
				// No obstruction between enemy and character
				return true;
			}
		}
	}

	// No enemies with line of sight
	return false;
}

void UOTDExposureHeatmapComponent::NotifyAIOfHighExposure()
{
	// In full implementation, this would:
	// 1. Query EQS to find artillery positions
	// 2. Trigger ranging shots that walk toward player position
	// 3. Increase sniper aggression
	// 4. Modify AI perception for easier spotting

	ArtilleryAttractedCount++;

	UE_LOG(LogTemp, Warning, TEXT("OTDExposure: High exposure attracted artillery! (Count: %d, Exposure: %.1f, Time: %.1fs)"),
		ArtilleryAttractedCount, CurrentExposure, TimeAtCurrentLevel);

	// Reset timer to prevent constant bombardment
	TimeAtCurrentLevel = 0.0f;

	// TODO: Trigger artillery event via gameplay tag or event
	// GameplayEventData.Instigator = GetOwner();
	// GameplayEventData.Target = GetOwner();
	// UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), FGameplayTag::RequestGameplayTag("Event.Artillery.Attracted"), GameplayEventData);
}
