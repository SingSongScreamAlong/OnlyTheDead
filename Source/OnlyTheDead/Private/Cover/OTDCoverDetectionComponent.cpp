// Only The Dead - Cover Detection Component Implementation

#include "Cover/OTDCoverDetectionComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

UOTDCoverDetectionComponent::UOTDCoverDetectionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Default settings
	CoverDetectionRange = 100.0f; // 1 meter
	DetectionInterval = 0.1f;     // 10 times per second
	TraceFanCount = 5;            // 5 traces in fan pattern
	bDebugDrawTraces = false;

	// Initialize protection values from design doc
	ShallowTrenchProtection = 0.60f;
	ShellCraterProtection = 0.75f;
	DugoutEntranceProtection = 0.85f;
	DeepDugoutProtection = 0.95f;
	ConcreteBunkerProtection = 0.99f;
}

void UOTDCoverDetectionComponent::BeginPlay()
{
	Super::BeginPlay();

	TimeSinceLastDetection = 0.0f;
}

void UOTDCoverDetectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceLastDetection += DeltaTime;

	// Periodic detection update
	if (TimeSinceLastDetection >= DetectionInterval)
	{
		PerformCoverDetection();
		TimeSinceLastDetection = 0.0f;
	}
}

// ========== Manual Query ==========

FOTDCoverInfo UOTDCoverDetectionComponent::DetectCoverAtLocation(const FVector& Location, const FRotator& Facing)
{
	const FVector Forward = Facing.Vector();
	const TArray<FHitResult> HitResults = PerformTraceFan(Location, Forward, CoverDetectionRange);

	return EvaluateCoverSurface(HitResults);
}

bool UOTDCoverDetectionComponent::HasOverheadProtection(const FVector& Location) const
{
	return CheckOverheadProtection(Location);
}

// ========== Internal Methods ==========

void UOTDCoverDetectionComponent::PerformCoverDetection()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character || !GetWorld())
	{
		return;
	}

	// Get character location and facing
	const FVector CharacterLocation = Character->GetActorLocation();
	const FVector Forward = Character->GetActorForwardVector();

	// Perform trace fan
	const TArray<FHitResult> HitResults = PerformTraceFan(CharacterLocation, Forward, CoverDetectionRange);

	// Evaluate cover
	PreviousCoverInfo = CurrentCoverInfo;
	CurrentCoverInfo = EvaluateCoverSurface(HitResults);

	// Check for overhead protection
	if (CurrentCoverInfo.CoverType != EOTDCoverType::None)
	{
		if (CheckOverheadProtection(CharacterLocation))
		{
			// Upgrade cover type if it has overhead protection
			if (CurrentCoverInfo.CoverType == EOTDCoverType::ShallowTrench)
			{
				CurrentCoverInfo.CoverType = EOTDCoverType::DugoutEntrance;
				CurrentCoverInfo.ProtectionValue = DugoutEntranceProtection;
			}
		}
	}

	// Broadcast state changes
	BroadcastCoverChange();
}

FOTDCoverInfo UOTDCoverDetectionComponent::EvaluateCoverSurface(const TArray<FHitResult>& HitResults) const
{
	FOTDCoverInfo CoverInfo;

	if (HitResults.Num() == 0)
	{
		return CoverInfo; // No cover
	}

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return CoverInfo;
	}

	const float CharacterHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.0f;

	// Find best cover from hit results
	float BestProtection = 0.0f;
	FHitResult BestHit;

	for (const FHitResult& Hit : HitResults)
	{
		if (IsSuitableCoverSurface(Hit, CharacterHeight))
		{
			const EOTDCoverType HitCoverType = DetermineCoverType(Hit);
			const float HitProtection = GetProtectionForCoverType(HitCoverType);

			if (HitProtection > BestProtection)
			{
				BestProtection = HitProtection;
				BestHit = Hit;
				CoverInfo.CoverType = HitCoverType;
			}
		}
	}

	// Populate cover info if we found suitable cover
	if (BestProtection > 0.0f)
	{
		CoverInfo.ProtectionValue = BestProtection;
		CoverInfo.CoverNormal = BestHit.Normal;
		CoverInfo.CoverLocation = BestHit.Location;
		CoverInfo.DistanceToCover = BestHit.Distance;

		// Determine cover height
		const float CoverHeightValue = BestHit.Location.Z - Character->GetActorLocation().Z;
		if (CoverHeightValue < CharacterHeight * 0.4f)
		{
			CoverInfo.CoverHeight = EOTDCoverHeight::Low;
		}
		else if (CoverHeightValue < CharacterHeight * 0.7f)
		{
			CoverInfo.CoverHeight = EOTDCoverHeight::High;
		}

		// Can peek if cover is high enough but not too high
		CoverInfo.bCanPeek = (CoverInfo.CoverHeight == EOTDCoverHeight::High);

		// Check if degraded (would require additional data, stubbed for now)
		CoverInfo.bIsDegraded = false;
	}

	return CoverInfo;
}

EOTDCoverType UOTDCoverDetectionComponent::DetermineCoverType(const FHitResult& HitResult) const
{
	// Check for gameplay tags on hit actor
	if (AActor* HitActor = HitResult.GetActor())
	{
		// Check for tagged objects (sandbags, revetments, bunkers, etc.)
		const FName ActorTag = HitActor->Tags.Num() > 0 ? HitActor->Tags[0] : NAME_None;

		if (ActorTag == FName("ConcreteBunker"))
		{
			return EOTDCoverType::ConcreteBunker;
		}
		else if (ActorTag == FName("DeepDugout"))
		{
			return EOTDCoverType::DeepDugout;
		}
		else if (ActorTag == FName("DugoutEntrance"))
		{
			return EOTDCoverType::DugoutEntrance;
		}
		else if (ActorTag == FName("ShellCrater"))
		{
			return EOTDCoverType::ShellCrater;
		}
		else if (ActorTag == FName("Trench") || ActorTag == FName("Sandbags"))
		{
			return EOTDCoverType::ShallowTrench;
		}
	}

	// Fallback to geometry-based detection
	// If hit surface is roughly vertical and below character, assume trench
	const float SurfaceAngle = FMath::Abs(FVector::DotProduct(HitResult.Normal, FVector::UpVector));

	if (SurfaceAngle < 0.3f) // Roughly vertical (< 17 degrees from vertical)
	{
		// Check if it's below ground level (crater-like)
		ACharacter* Character = Cast<ACharacter>(GetOwner());
		if (Character && HitResult.Location.Z < Character->GetActorLocation().Z - 50.0f)
		{
			return EOTDCoverType::ShellCrater;
		}

		return EOTDCoverType::ShallowTrench;
	}

	return EOTDCoverType::None;
}

float UOTDCoverDetectionComponent::GetProtectionForCoverType(EOTDCoverType CoverType) const
{
	switch (CoverType)
	{
	case EOTDCoverType::ShallowTrench:
		return ShallowTrenchProtection;
	case EOTDCoverType::ShellCrater:
		return ShellCraterProtection;
	case EOTDCoverType::DugoutEntrance:
		return DugoutEntranceProtection;
	case EOTDCoverType::DeepDugout:
		return DeepDugoutProtection;
	case EOTDCoverType::ConcreteBunker:
		return ConcreteBunkerProtection;
	default:
		return 0.0f;
	}
}

bool UOTDCoverDetectionComponent::IsSuitableCoverSurface(const FHitResult& HitResult, float CharacterHeight) const
{
	// Must have hit something
	if (!HitResult.bBlockingHit)
	{
		return false;
	}

	// Check if surface is roughly vertical (good for cover)
	const float SurfaceAngle = FMath::Abs(FVector::DotProduct(HitResult.Normal, FVector::UpVector));

	// Surface should be mostly vertical (< 45 degrees from vertical)
	if (SurfaceAngle > 0.707f) // cos(45°)
	{
		return false;
	}

	// Check height is appropriate (between knee and head)
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return false;
	}

	const float HeightDiff = HitResult.Location.Z - Character->GetActorLocation().Z;
	const float MinHeight = CharacterHeight * 0.3f; // Knee height
	const float MaxHeight = CharacterHeight * 1.2f; // Above head

	return (HeightDiff >= -CharacterHeight && HeightDiff <= MaxHeight);
}

TArray<FHitResult> UOTDCoverDetectionComponent::PerformTraceFan(const FVector& Start, const FVector& Forward, float Range) const
{
	TArray<FHitResult> HitResults;

	if (!GetWorld())
	{
		return HitResults;
	}

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return HitResults;
	}

	const float CharacterHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.0f;

	// Trace at three heights: chest, waist, knee
	TArray<float> TraceHeights = {
		CharacterHeight * 0.7f,  // Chest
		CharacterHeight * 0.5f,  // Waist
		CharacterHeight * 0.3f   // Knee
	};

	// Fan out traces horizontally
	const float FanAngle = 60.0f; // Total fan spread in degrees
	const float AngleStep = FanAngle / FMath::Max(1, TraceFanCount - 1);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);

	for (float Height : TraceHeights)
	{
		const FVector TraceStart = Start + FVector(0.0f, 0.0f, Height - CharacterHeight * 0.5f);

		for (int32 i = 0; i < TraceFanCount; ++i)
		{
			// Calculate trace direction
			const float Angle = -FanAngle * 0.5f + (i * AngleStep);
			const FVector Direction = Forward.RotateAngleAxis(Angle, FVector::UpVector);
			const FVector TraceEnd = TraceStart + Direction * Range;

			// Perform trace
			FHitResult HitResult;
			if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
			{
				HitResults.Add(HitResult);

				// Debug visualization
				if (bDebugDrawTraces)
				{
					DrawDebugLine(GetWorld(), TraceStart, HitResult.Location, FColor::Green, false, DetectionInterval);
					DrawDebugPoint(GetWorld(), HitResult.Location, 5.0f, FColor::Red, false, DetectionInterval);
				}
			}
			else if (bDebugDrawTraces)
			{
				DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, DetectionInterval);
			}
		}
	}

	return HitResults;
}

bool UOTDCoverDetectionComponent::CheckOverheadProtection(const FVector& Location) const
{
	if (!GetWorld())
	{
		return false;
	}

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return false;
	}

	// Trace upward to check for roof/ceiling
	const FVector TraceStart = Location;
	const FVector TraceEnd = Location + FVector(0.0f, 0.0f, 300.0f); // 3 meters up

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		// Check if hit is tagged as overhead protection
		if (AActor* HitActor = HitResult.GetActor())
		{
			for (const FName& Tag : HitActor->Tags)
			{
				if (Tag == FName("DugoutRoof") || Tag == FName("BunkerRoof") || Tag == FName("Overhead"))
				{
					return true;
				}
			}
		}

		// Also consider low ceilings as overhead protection (dugouts, tunnels)
		if (HitResult.Distance < 200.0f) // Less than 2 meters overhead
		{
			return true;
		}
	}

	return false;
}

void UOTDCoverDetectionComponent::BroadcastCoverChange()
{
	// Check if cover state changed
	const bool bWasInCover = (PreviousCoverInfo.CoverType != EOTDCoverType::None);
	const bool bIsNowInCover = (CurrentCoverInfo.CoverType != EOTDCoverType::None);

	if (!bWasInCover && bIsNowInCover)
	{
		// Entered cover
		OnEnteredCover.Broadcast(CurrentCoverInfo);
		UE_LOG(LogTemp, Log, TEXT("OTDCover: Entered cover type %d (%.0f%% protection)"),
			static_cast<int32>(CurrentCoverInfo.CoverType), CurrentCoverInfo.ProtectionValue * 100.0f);
	}
	else if (bWasInCover && !bIsNowInCover)
	{
		// Exited cover
		OnExitedCover.Broadcast(CurrentCoverInfo);
		UE_LOG(LogTemp, Log, TEXT("OTDCover: Exited cover"));
	}
	else if (bWasInCover && bIsNowInCover && CurrentCoverInfo.CoverType != PreviousCoverInfo.CoverType)
	{
		// Cover type changed
		OnCoverChanged.Broadcast(CurrentCoverInfo);
		UE_LOG(LogTemp, Log, TEXT("OTDCover: Cover changed to type %d (%.0f%% protection)"),
			static_cast<int32>(CurrentCoverInfo.CoverType), CurrentCoverInfo.ProtectionValue * 100.0f);
	}
}
