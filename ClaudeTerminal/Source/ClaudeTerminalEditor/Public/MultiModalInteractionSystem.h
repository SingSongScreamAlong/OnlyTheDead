// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"

DECLARE_DELEGATE_TwoParams(FOnMultiModalCommand, const FString&, const TArray<FVector>&);

/**
 * Input modality
 */
UENUM()
enum class EInputModality : uint8
{
	Voice,      // Speech input
	Click,      // Mouse clicks
	Type,       // Keyboard text
	Gesture,    // Mouse gestures
	Hybrid      // Combined modalities
};

/**
 * Multi-modal command
 */
struct FMultiModalCommand
{
	FString TextPart;                    // Typed or transcribed text
	TArray<FVector> ClickedLocations;    // Viewport clicks
	TArray<FVector> GesturePoints;       // Mouse gesture path
	EInputModality PrimaryModality;
	double Timestamp;
};

/**
 * Gesture recognition result
 */
struct FGestureRecognition
{
	FString GestureType;  // "circle", "line", "zigzag"
	float Confidence;     // 0.0-1.0
	TArray<FVector> Points;
};

/**
 * Multi-modal interaction system combining voice, click, gesture, and text
 * TIER 4.1 Feature - Future Vision
 */
class CLAUDETERMINALEDITOR_API FMultiModalInteractionSystem
{
public:
	FMultiModalInteractionSystem();
	~FMultiModalInteractionSystem();

	/**
	 * Initialize multi-modal system
	 */
	bool Initialize();

	/**
	 * Shutdown system
	 */
	void Shutdown();

	/**
	 * Start multi-modal input mode
	 * @param OnCommandComplete Callback when command is complete
	 * @return true if started successfully
	 */
	bool StartMultiModalInput(FOnMultiModalCommand OnCommandComplete);

	/**
	 * Cancel current multi-modal input
	 */
	void CancelInput();

	/**
	 * Add voice input to current command
	 * @param TranscribedText Voice-to-text result
	 */
	void AddVoiceInput(const FString& TranscribedText);

	/**
	 * Add click location to current command
	 * @param WorldLocation Clicked location in world space
	 */
	void AddClickInput(const FVector& WorldLocation);

	/**
	 * Add text input to current command
	 * @param Text Typed text
	 */
	void AddTextInput(const FString& Text);

	/**
	 * Start gesture recording
	 */
	void StartGestureRecording();

	/**
	 * Add point to gesture
	 * @param Point World location
	 */
	void AddGesturePoint(const FVector& Point);

	/**
	 * End gesture recording and recognize
	 * @param OutGesture Recognized gesture
	 * @return true if gesture recognized
	 */
	bool EndGestureRecording(FGestureRecognition& OutGesture);

	/**
	 * Complete current multi-modal command
	 */
	void CompleteCommand();

	/**
	 * Check if currently in multi-modal input mode
	 */
	bool IsActive() const { return bIsActive; }

	/**
	 * Get current command being built
	 */
	const FMultiModalCommand& GetCurrentCommand() const { return CurrentCommand; }

	/**
	 * Recognize gesture from points
	 * @param Points Gesture path points
	 * @param OutGesture Recognized gesture
	 * @return true if recognized
	 */
	static bool RecognizeGesture(const TArray<FVector>& Points, FGestureRecognition& OutGesture);

	/**
	 * Combine multi-modal inputs into single natural language command
	 * @param Command Multi-modal command
	 * @return Combined natural language string
	 */
	static FString CombineInputs(const FMultiModalCommand& Command);

private:
	// Gesture recognition helpers
	static bool IsCircleGesture(const TArray<FVector>& Points, float& OutConfidence);
	static bool IsLineGesture(const TArray<FVector>& Points, float& OutConfidence);
	static bool IsZigzagGesture(const TArray<FVector>& Points, float& OutConfidence);

	// Calculate gesture metrics
	static float CalculateTotalDistance(const TArray<FVector>& Points);
	static float CalculateLinearity(const TArray<FVector>& Points);
	static FVector CalculateCentroid(const TArray<FVector>& Points);

	// State
	bool bIsActive;
	bool bGestureRecording;
	FMultiModalCommand CurrentCommand;
	TArray<FVector> CurrentGesturePoints;

	// Delegates
	FOnMultiModalCommand OnCommandCompleteCallback;
};
