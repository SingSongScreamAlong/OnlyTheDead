// Copyright Only The Dead Development Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LLMIntegration.h"
#include "VisionSystem.generated.h"

/**
 * Screenshot capture settings
 */
USTRUCT(BlueprintType)
struct FScreenshotSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vision")
	int32 Width = 1920;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vision")
	int32 Height = 1080;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vision")
	bool bIncludeUI = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vision")
	FString OutputPath;
};

/**
 * Visual analysis result
 */
USTRUCT(BlueprintType)
struct FVisualAnalysisResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vision")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vision")
	TArray<FString> DetectedObjects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vision")
	TArray<FString> Issues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vision")
	TArray<FString> Suggestions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vision")
	float QualityScore = 0.0f; // 0-1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vision")
	bool bMatchesRequirements = false;
};

DECLARE_DELEGATE_OneParam(FOnVisualAnalysisComplete, const FVisualAnalysisResult& /* Result */);
DECLARE_DELEGATE_OneParam(FOnVisionFailed, const FString& /* Error */);

/**
 * Vision System - Allows AI to "see" what it's building
 * Uses multimodal LLMs (Claude 3.5 Sonnet, GPT-4 Vision) to analyze screenshots
 */
UCLASS(BlueprintType)
class VERDUNAIAGENT_API UVisionSystem : public UObject
{
	GENERATED_BODY()

public:
	UVisionSystem();

	/**
	 * Capture a screenshot of the current viewport
	 * @return Path to saved screenshot, or empty string on failure
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Vision")
	static FString CaptureViewportScreenshot(const FScreenshotSettings& Settings);

	/**
	 * Capture screenshot of a specific actor
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Vision")
	static FString CaptureActorScreenshot(AActor* Actor, const FScreenshotSettings& Settings);

	/**
	 * Capture screenshot from a specific camera position
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Vision")
	static FString CaptureFromPosition(const FVector& Position, const FRotator& Rotation, const FScreenshotSettings& Settings);

	/**
	 * Analyze a screenshot using multimodal LLM
	 * @param ImagePath - Path to image file
	 * @param Question - What to analyze (e.g., "Does this look like a WWI trench?")
	 * @param Config - LLM configuration
	 */
	void AnalyzeImage(
		const FString& ImagePath,
		const FString& Question,
		const FLLMRequestConfig& Config,
		FOnVisualAnalysisComplete OnComplete,
		FOnVisionFailed OnFailed
	);

	/**
	 * Verify that built content matches requirements
	 * @param Requirements - What should be in the image
	 * @param ImagePath - Screenshot to verify
	 */
	void VerifyAgainstRequirements(
		const FString& Requirements,
		const FString& ImagePath,
		const FLLMRequestConfig& Config,
		FOnVisualAnalysisComplete OnComplete,
		FOnVisionFailed OnFailed
	);

	/**
	 * Compare two screenshots
	 * @param BeforeImagePath - Image before changes
	 * @param AfterImagePath - Image after changes
	 * @return Description of differences
	 */
	void CompareScreenshots(
		const FString& BeforeImagePath,
		const FString& AfterImagePath,
		const FString& Question,
		const FLLMRequestConfig& Config,
		FOnVisualAnalysisComplete OnComplete,
		FOnVisionFailed OnFailed
	);

	/**
	 * Get AI suggestions for improvement based on image
	 */
	void GetImprovementSuggestions(
		const FString& ImagePath,
		const FString& Context,
		const FLLMRequestConfig& Config,
		FOnVisualAnalysisComplete OnComplete,
		FOnVisionFailed OnFailed
	);

	/**
	 * Quick verification: "Does this look correct?"
	 */
	void QuickVerification(
		const FString& ImagePath,
		const FString& ExpectedResult,
		const FLLMRequestConfig& Config,
		FOnVisualAnalysisComplete OnComplete,
		FOnVisionFailed OnFailed
	);

private:
	// Image encoding
	FString EncodeImageToBase64(const FString& ImagePath);

	// Build multimodal request
	FString BuildMultimodalRequest(
		const FString& ImageBase64,
		const FString& Prompt,
		const FLLMRequestConfig& Config
	);

	// Parse response
	FVisualAnalysisResult ParseVisualResponse(const FString& Response);

	// Callbacks
	void OnAnalysisResponseReceived(const FString& Response);
	void OnAnalysisFailed(const FString& Error);

	// Member variables
	UPROPERTY()
	ULLMIntegration* LLMIntegration;

	FOnVisualAnalysisComplete CurrentSuccessCallback;
	FOnVisionFailed CurrentFailureCallback;
	FString CurrentPrompt;
};

/**
 * Vision-enabled verification system
 * Allows autonomous agents to verify their work visually
 */
UCLASS(BlueprintType)
class VERDUNAIAGENT_API UVisualVerificationSystem : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Automated visual verification workflow
	 * 1. Capture screenshot
	 * 2. Analyze with AI
	 * 3. Return verification result
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Vision")
	void VerifyWorkAutomatically(
		const FString& TaskDescription,
		const FString& Requirements,
		const FLLMRequestConfig& Config,
		FOnVisualAnalysisComplete OnComplete,
		FOnVisionFailed OnFailed
	);

	/**
	 * Multi-angle verification (capture from multiple viewpoints)
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Vision")
	void VerifyFromMultipleAngles(
		const FVector& TargetLocation,
		const TArray<FRotator>& CameraAngles,
		const FString& Requirements,
		const FLLMRequestConfig& Config,
		FOnVisualAnalysisComplete OnComplete,
		FOnVisionFailed OnFailed
	);

	/**
	 * Verify specific actor
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Agent|Vision")
	void VerifyActor(
		AActor* Actor,
		const FString& Requirements,
		const FLLMRequestConfig& Config,
		FOnVisualAnalysisComplete OnComplete,
		FOnVisionFailed OnFailed
	);

private:
	UPROPERTY()
	UVisionSystem* VisionSystem;
};
