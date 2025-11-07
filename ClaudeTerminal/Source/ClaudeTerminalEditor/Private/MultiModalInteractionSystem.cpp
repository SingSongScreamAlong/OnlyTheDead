// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiModalInteractionSystem.h"
#include "Kismet/KismetMathLibrary.h"

FMultiModalInteractionSystem::FMultiModalInteractionSystem()
	: bIsActive(false)
	, bGestureRecording(false)
{
}

FMultiModalInteractionSystem::~FMultiModalInteractionSystem()
{
	Shutdown();
}

bool FMultiModalInteractionSystem::Initialize()
{
	UE_LOG(LogTemp, Log, TEXT("MultiModal: Initializing system"));

	bIsActive = false;
	bGestureRecording = false;

	return true;
}

void FMultiModalInteractionSystem::Shutdown()
{
	if (bIsActive)
	{
		CancelInput();
	}
}

bool FMultiModalInteractionSystem::StartMultiModalInput(FOnMultiModalCommand OnCommandComplete)
{
	if (bIsActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("MultiModal: Already active"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("MultiModal: Starting multi-modal input mode"));

	bIsActive = true;
	OnCommandCompleteCallback = OnCommandComplete;

	// Reset current command
	CurrentCommand = FMultiModalCommand();
	CurrentCommand.Timestamp = FPlatformTime::Seconds();
	CurrentCommand.PrimaryModality = EInputModality::Hybrid;

	CurrentGesturePoints.Empty();

	return true;
}

void FMultiModalInteractionSystem::CancelInput()
{
	UE_LOG(LogTemp, Log, TEXT("MultiModal: Cancelled"));

	bIsActive = false;
	bGestureRecording = false;
	CurrentCommand = FMultiModalCommand();
	CurrentGesturePoints.Empty();
	OnCommandCompleteCallback.Unbind();
}

void FMultiModalInteractionSystem::AddVoiceInput(const FString& TranscribedText)
{
	if (!bIsActive)
		return;

	UE_LOG(LogTemp, Log, TEXT("MultiModal: Voice input: '%s'"), *TranscribedText);

	if (CurrentCommand.TextPart.IsEmpty())
	{
		CurrentCommand.TextPart = TranscribedText;
	}
	else
	{
		CurrentCommand.TextPart += TEXT(" ") + TranscribedText;
	}

	CurrentCommand.PrimaryModality = EInputModality::Voice;
}

void FMultiModalInteractionSystem::AddClickInput(const FVector& WorldLocation)
{
	if (!bIsActive)
		return;

	UE_LOG(LogTemp, Log, TEXT("MultiModal: Click at: %s"), *WorldLocation.ToString());

	CurrentCommand.ClickedLocations.Add(WorldLocation);

	if (CurrentCommand.PrimaryModality != EInputModality::Voice)
	{
		CurrentCommand.PrimaryModality = EInputModality::Click;
	}
}

void FMultiModalInteractionSystem::AddTextInput(const FString& Text)
{
	if (!bIsActive)
		return;

	UE_LOG(LogTemp, Log, TEXT("MultiModal: Text input: '%s'"), *Text);

	if (CurrentCommand.TextPart.IsEmpty())
	{
		CurrentCommand.TextPart = Text;
	}
	else
	{
		CurrentCommand.TextPart += TEXT(" ") + Text;
	}

	if (CurrentCommand.PrimaryModality != EInputModality::Voice)
	{
		CurrentCommand.PrimaryModality = EInputModality::Type;
	}
}

void FMultiModalInteractionSystem::StartGestureRecording()
{
	if (!bIsActive)
		return;

	UE_LOG(LogTemp, Log, TEXT("MultiModal: Started gesture recording"));

	bGestureRecording = true;
	CurrentGesturePoints.Empty();
}

void FMultiModalInteractionSystem::AddGesturePoint(const FVector& Point)
{
	if (!bIsActive || !bGestureRecording)
		return;

	CurrentGesturePoints.Add(Point);
}

bool FMultiModalInteractionSystem::EndGestureRecording(FGestureRecognition& OutGesture)
{
	if (!bIsActive || !bGestureRecording)
		return false;

	bGestureRecording = false;

	UE_LOG(LogTemp, Log, TEXT("MultiModal: Ending gesture recording (%d points)"), CurrentGesturePoints.Num());

	if (RecognizeGesture(CurrentGesturePoints, OutGesture))
	{
		CurrentCommand.GesturePoints = CurrentGesturePoints;
		CurrentCommand.PrimaryModality = EInputModality::Gesture;

		UE_LOG(LogTemp, Log, TEXT("MultiModal: Recognized gesture: %s (confidence: %.2f)"),
			*OutGesture.GestureType, OutGesture.Confidence);

		return true;
	}

	return false;
}

void FMultiModalInteractionSystem::CompleteCommand()
{
	if (!bIsActive)
		return;

	UE_LOG(LogTemp, Log, TEXT("MultiModal: Completing command"));

	if (OnCommandCompleteCallback.IsBound())
	{
		// Combine all inputs
		FString FinalCommand = CombineInputs(CurrentCommand);

		OnCommandCompleteCallback.Execute(FinalCommand, CurrentCommand.ClickedLocations);
	}

	// Reset for next command
	bIsActive = false;
	CurrentCommand = FMultiModalCommand();
	CurrentGesturePoints.Empty();
}

bool FMultiModalInteractionSystem::RecognizeGesture(
	const TArray<FVector>& Points,
	FGestureRecognition& OutGesture)
{
	if (Points.Num() < 3)
	{
		return false;
	}

	OutGesture.Points = Points;

	// Try to recognize different gesture types
	float CircleConfidence = 0.0f;
	float LineConfidence = 0.0f;
	float ZigzagConfidence = 0.0f;

	IsCircleGesture(Points, CircleConfidence);
	IsLineGesture(Points, LineConfidence);
	IsZigzagGesture(Points, ZigzagConfidence);

	// Select gesture with highest confidence
	float MaxConfidence = FMath::Max3(CircleConfidence, LineConfidence, ZigzagConfidence);

	if (MaxConfidence < 0.5f)
	{
		return false; // No confident match
	}

	if (CircleConfidence == MaxConfidence)
	{
		OutGesture.GestureType = TEXT("circle");
		OutGesture.Confidence = CircleConfidence;
	}
	else if (LineConfidence == MaxConfidence)
	{
		OutGesture.GestureType = TEXT("line");
		OutGesture.Confidence = LineConfidence;
	}
	else
	{
		OutGesture.GestureType = TEXT("zigzag");
		OutGesture.Confidence = ZigzagConfidence;
	}

	return true;
}

FString FMultiModalInteractionSystem::CombineInputs(const FMultiModalCommand& Command)
{
	FString Combined;

	// Start with text part
	if (!Command.TextPart.IsEmpty())
	{
		Combined = Command.TextPart;
	}

	// Add click information
	if (Command.ClickedLocations.Num() > 0)
	{
		if (!Combined.IsEmpty())
		{
			Combined += TEXT(" ");
		}

		if (Command.ClickedLocations.Num() == 1)
		{
			FVector Loc = Command.ClickedLocations[0];
			Combined += FString::Printf(TEXT("at location %.0f, %.0f, %.0f"),
				Loc.X, Loc.Y, Loc.Z);
		}
		else
		{
			Combined += FString::Printf(TEXT("at %d clicked locations"),
				Command.ClickedLocations.Num());
		}
	}

	// Add gesture information
	if (Command.GesturePoints.Num() > 0)
	{
		FGestureRecognition Gesture;
		if (RecognizeGesture(Command.GesturePoints, Gesture))
		{
			if (!Combined.IsEmpty())
			{
				Combined += TEXT(" ");
			}

			Combined += FString::Printf(TEXT("in %s pattern"), *Gesture.GestureType);
		}
	}

	return Combined;
}

bool FMultiModalInteractionSystem::IsCircleGesture(const TArray<FVector>& Points, float& OutConfidence)
{
	if (Points.Num() < 8)
	{
		OutConfidence = 0.0f;
		return false;
	}

	// Calculate centroid
	FVector Centroid = CalculateCentroid(Points);

	// Calculate average distance from centroid
	float AvgDistance = 0.0f;
	for (const FVector& Point : Points)
	{
		AvgDistance += FVector::Dist(Point, Centroid);
	}
	AvgDistance /= Points.Num();

	// Calculate variance in distance (low variance = more circular)
	float Variance = 0.0f;
	for (const FVector& Point : Points)
	{
		float Distance = FVector::Dist(Point, Centroid);
		float Diff = Distance - AvgDistance;
		Variance += Diff * Diff;
	}
	Variance /= Points.Num();

	// Check if first and last points are close (closed loop)
	float ClosureDistance = FVector::Dist(Points[0], Points.Last());

	// Calculate confidence
	float VarianceScore = FMath::Clamp(1.0f - (Variance / (AvgDistance * AvgDistance)), 0.0f, 1.0f);
	float ClosureScore = FMath::Clamp(1.0f - (ClosureDistance / AvgDistance), 0.0f, 1.0f);

	OutConfidence = (VarianceScore * 0.7f + ClosureScore * 0.3f);

	return OutConfidence > 0.5f;
}

bool FMultiModalInteractionSystem::IsLineGesture(const TArray<FVector>& Points, float& OutConfidence)
{
	if (Points.Num() < 3)
	{
		OutConfidence = 0.0f;
		return false;
	}

	// Calculate linearity
	float Linearity = CalculateLinearity(Points);

	OutConfidence = Linearity;

	return OutConfidence > 0.7f;
}

bool FMultiModalInteractionSystem::IsZigzagGesture(const TArray<FVector>& Points, float& OutConfidence)
{
	if (Points.Num() < 6)
	{
		OutConfidence = 0.0f;
		return false;
	}

	// Count direction changes
	int32 DirectionChanges = 0;
	FVector PrevDirection = (Points[1] - Points[0]).GetSafeNormal();

	for (int32 i = 2; i < Points.Num(); i++)
	{
		FVector CurrentDirection = (Points[i] - Points[i - 1]).GetSafeNormal();
		float DotProduct = FVector::DotProduct(PrevDirection, CurrentDirection);

		// Direction changed significantly
		if (DotProduct < 0.5f)
		{
			DirectionChanges++;
		}

		PrevDirection = CurrentDirection;
	}

	// Zigzag has multiple direction changes
	float ChangeRatio = (float)DirectionChanges / Points.Num();

	OutConfidence = FMath::Clamp(ChangeRatio * 2.0f, 0.0f, 1.0f);

	return OutConfidence > 0.5f && DirectionChanges >= 3;
}

float FMultiModalInteractionSystem::CalculateTotalDistance(const TArray<FVector>& Points)
{
	float TotalDistance = 0.0f;

	for (int32 i = 1; i < Points.Num(); i++)
	{
		TotalDistance += FVector::Dist(Points[i], Points[i - 1]);
	}

	return TotalDistance;
}

float FMultiModalInteractionSystem::CalculateLinearity(const TArray<FVector>& Points)
{
	if (Points.Num() < 3)
		return 0.0f;

	// Calculate straight-line distance vs. path distance
	float StraightDistance = FVector::Dist(Points[0], Points.Last());
	float PathDistance = CalculateTotalDistance(Points);

	if (PathDistance < 0.01f)
		return 0.0f;

	float Linearity = StraightDistance / PathDistance;

	return Linearity;
}

FVector FMultiModalInteractionSystem::CalculateCentroid(const TArray<FVector>& Points)
{
	FVector Sum = FVector::ZeroVector;

	for (const FVector& Point : Points)
	{
		Sum += Point;
	}

	return Sum / Points.Num();
}

/*
 * MULTI-MODAL INTERACTION EXAMPLES:
 *
 * Example 1: Voice + Click
 * User: *Holds V* "Place buildings"
 * System: "Click locations in viewport"
 * User: *Clicks 5 locations*
 * Result: "Place buildings at 5 clicked locations"
 *
 * Example 2: Type + Gesture
 * User: *Types* "Create trench"
 * User: *Draws zigzag gesture in viewport*
 * Result: "Create trench in zigzag pattern"
 *
 * Example 3: Voice + Click + Type
 * User: *Holds V* "Add sandbags"
 * User: *Clicks perimeter locations*
 * User: *Types* "facing outward"
 * Result: "Add sandbags at clicked locations facing outward"
 *
 * Example 4: Gesture Only
 * User: *Draws circle gesture*
 * System: "What would you like to place in circle?"
 * User: "Trees"
 * Result: "Place trees in circle pattern"
 */
