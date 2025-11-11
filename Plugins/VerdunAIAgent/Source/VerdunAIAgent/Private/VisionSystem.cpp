// Copyright Only The Dead Development Team. All Rights Reserved.

#include "VisionSystem.h"
#include "Editor.h"
#include "EditorViewportClient.h"
#include "LevelEditor.h"
#include "LevelEditorViewport.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Base64.h"
#include "ImageUtils.h"
#include "Engine/SceneCapture2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Json.h"
#include "JsonUtilities.h"

UVisionSystem::UVisionSystem()
{
	LLMIntegration = NewObject<ULLMIntegration>(this);
}

FString UVisionSystem::CaptureViewportScreenshot(const FScreenshotSettings& Settings)
{
	if (!GEditor)
	{
		UE_LOG(LogTemp, Error, TEXT("No editor available for screenshot"));
		return FString();
	}

	// Get active viewport
	FViewport* Viewport = GEditor->GetActiveViewport();
	if (!Viewport)
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
		TSharedPtr<SLevelViewport> ActiveLevelViewport = LevelEditorModule.GetFirstActiveLevelViewport();
		if (ActiveLevelViewport.IsValid())
		{
			Viewport = ActiveLevelViewport->GetActiveViewport();
		}
	}

	if (!Viewport)
	{
		UE_LOG(LogTemp, Error, TEXT("No active viewport found"));
		return FString();
	}

	// Capture screenshot
	TArray<FColor> Bitmap;
	FIntVector Size(Settings.Width, Settings.Height, 0);

	if (!Viewport->ReadPixels(Bitmap, FReadSurfaceDataFlags(), FIntRect(0, 0, Settings.Width, Settings.Height)))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read viewport pixels"));
		return FString();
	}

	// Determine output path
	FString OutputPath = Settings.OutputPath;
	if (OutputPath.IsEmpty())
	{
		FString ProjectDir = FPaths::ProjectSavedDir();
		FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
		OutputPath = ProjectDir / TEXT("Screenshots") / FString::Printf(TEXT("AIAgent_%s.png"), *Timestamp);
	}

	// Ensure directory exists
	FString Directory = FPaths::GetPath(OutputPath);
	if (!FPaths::DirectoryExists(Directory))
	{
		IFileManager::Get().MakeDirectory(*Directory, true);
	}

	// Save as PNG
	FFileHelper::CreateBitmap(*OutputPath, Settings.Width, Settings.Height, Bitmap.GetData());

	UE_LOG(LogTemp, Log, TEXT("Screenshot saved: %s"), *OutputPath);

	return OutputPath;
}

FString UVisionSystem::CaptureActorScreenshot(AActor* Actor, const FScreenshotSettings& Settings)
{
	if (!Actor)
	{
		UE_LOG(LogTemp, Error, TEXT("No actor provided for screenshot"));
		return FString();
	}

	// Get actor bounds
	FVector Origin, Extent;
	Actor->GetActorBounds(false, Origin, Extent);

	// Calculate camera position to frame the actor
	float Distance = Extent.Size() * 1.5f;
	FVector CameraPos = Origin + FVector(-Distance, 0, Distance * 0.5f);
	FRotator CameraRot = (Origin - CameraPos).Rotation();

	return CaptureFromPosition(CameraPos, CameraRot, Settings);
}

FString UVisionSystem::CaptureFromPosition(const FVector& Position, const FRotator& Rotation, const FScreenshotSettings& Settings)
{
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("No world for screenshot"));
		return FString();
	}

	// Create temporary scene capture
	ASceneCapture2D* SceneCapture = World->SpawnActor<ASceneCapture2D>(Position, Rotation);
	if (!SceneCapture)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create scene capture"));
		return FString();
	}

	// Configure capture component
	USceneCaptureComponent2D* CaptureComponent = SceneCapture->GetCaptureComponent2D();
	CaptureComponent->CaptureSource = SCS_FinalColorLDR;
	CaptureComponent->bCaptureEveryFrame = false;
	CaptureComponent->bCaptureOnMovement = false;

	// Create render target
	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
	RenderTarget->InitAutoFormat(Settings.Width, Settings.Height);
	RenderTarget->UpdateResourceImmediate(true);

	CaptureComponent->TextureTarget = RenderTarget;
	CaptureComponent->CaptureScene();

	// Read pixels from render target
	TArray<FColor> Bitmap;
	FTextureRenderTargetResource* RTResource = RenderTarget->GameThread_GetRenderTargetResource();
	if (RTResource)
	{
		RTResource->ReadPixels(Bitmap);
	}

	// Clean up
	World->DestroyActor(SceneCapture);

	if (Bitmap.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read pixels from render target"));
		return FString();
	}

	// Save image
	FString OutputPath = Settings.OutputPath;
	if (OutputPath.IsEmpty())
	{
		FString ProjectDir = FPaths::ProjectSavedDir();
		FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
		OutputPath = ProjectDir / TEXT("Screenshots") / FString::Printf(TEXT("AIAgent_Capture_%s.png"), *Timestamp);
	}

	FString Directory = FPaths::GetPath(OutputPath);
	if (!FPaths::DirectoryExists(Directory))
	{
		IFileManager::Get().MakeDirectory(*Directory, true);
	}

	FFileHelper::CreateBitmap(*OutputPath, Settings.Width, Settings.Height, Bitmap.GetData());

	UE_LOG(LogTemp, Log, TEXT("Capture saved: %s"), *OutputPath);

	return OutputPath;
}

void UVisionSystem::AnalyzeImage(
	const FString& ImagePath,
	const FString& Question,
	const FLLMRequestConfig& Config,
	FOnVisualAnalysisComplete OnComplete,
	FOnVisionFailed OnFailed)
{
	if (!FPaths::FileExists(ImagePath))
	{
		OnFailed.ExecuteIfBound(FString::Printf(TEXT("Image file not found: %s"), *ImagePath));
		return;
	}

	CurrentSuccessCallback = OnComplete;
	CurrentFailureCallback = OnFailed;
	CurrentPrompt = Question;

	// Encode image to base64
	FString ImageBase64 = EncodeImageToBase64(ImagePath);
	if (ImageBase64.IsEmpty())
	{
		OnFailed.ExecuteIfBound(TEXT("Failed to encode image"));
		return;
	}

	// Build multimodal request
	FString Request = BuildMultimodalRequest(ImageBase64, Question, Config);

	// Send to LLM
	FOnLLMResponseReceived OnSuccess;
	OnSuccess.BindUObject(this, &UVisionSystem::OnAnalysisResponseReceived);

	FOnLLMRequestFailed OnFailure;
	OnFailure.BindUObject(this, &UVisionSystem::OnAnalysisFailed);

	LLMIntegration->SendRequest(Request, Config, OnSuccess, OnFailure);
}

void UVisionSystem::VerifyAgainstRequirements(
	const FString& Requirements,
	const FString& ImagePath,
	const FLLMRequestConfig& Config,
	FOnVisualAnalysisComplete OnComplete,
	FOnVisionFailed OnFailed)
{
	FString Question = FString::Printf(
		TEXT("Analyze this image and verify if it meets these requirements:\n\n%s\n\n"
		     "Respond with:\n"
		     "1. Whether it matches the requirements (YES/NO)\n"
		     "2. What objects you see\n"
		     "3. Any issues or discrepancies\n"
		     "4. Suggestions for improvement\n"
		     "5. Quality score (0-10)"),
		*Requirements
	);

	AnalyzeImage(ImagePath, Question, Config, OnComplete, OnFailed);
}

void UVisionSystem::CompareScreenshots(
	const FString& BeforeImagePath,
	const FString& AfterImagePath,
	const FString& Question,
	const FLLMRequestConfig& Config,
	FOnVisualAnalysisComplete OnComplete,
	FOnVisionFailed OnFailed)
{
	// TODO: Implement multi-image comparison
	// This requires sending multiple images to the LLM
	// For now, just analyze the "after" image

	FString ComparisonQuestion = FString::Printf(
		TEXT("Analyze this image (after changes) and describe what you see.\n\nContext: %s"),
		*Question
	);

	AnalyzeImage(AfterImagePath, ComparisonQuestion, Config, OnComplete, OnFailed);
}

void UVisionSystem::GetImprovementSuggestions(
	const FString& ImagePath,
	const FString& Context,
	const FLLMRequestConfig& Config,
	FOnVisualAnalysisComplete OnComplete,
	FOnVisionFailed OnFailed)
{
	FString Question = FString::Printf(
		TEXT("Analyze this image and provide specific suggestions for improvement.\n\n"
		     "Context: %s\n\n"
		     "Consider:\n"
		     "- Historical accuracy (if relevant)\n"
		     "- Visual quality\n"
		     "- Realism\n"
		     "- Details that could be added\n"
		     "- Anything that looks wrong or out of place"),
		*Context
	);

	AnalyzeImage(ImagePath, Question, Config, OnComplete, OnFailed);
}

void UVisionSystem::QuickVerification(
	const FString& ImagePath,
	const FString& ExpectedResult,
	const FLLMRequestConfig& Config,
	FOnVisualAnalysisComplete OnComplete,
	FOnVisionFailed OnFailed)
{
	FString Question = FString::Printf(
		TEXT("Quick verification: Does this image show %s? Answer YES or NO and briefly explain why."),
		*ExpectedResult
	);

	AnalyzeImage(ImagePath, Question, Config, OnComplete, OnFailed);
}

// ============================================================================
// PRIVATE IMPLEMENTATION
// ============================================================================

FString UVisionSystem::EncodeImageToBase64(const FString& ImagePath)
{
	// Read image file
	TArray<uint8> ImageData;
	if (!FFileHelper::LoadFileToArray(ImageData, *ImagePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read image file: %s"), *ImagePath);
		return FString();
	}

	// Encode to base64
	FString Base64 = FBase64::Encode(ImageData);
	return Base64;
}

FString UVisionSystem::BuildMultimodalRequest(
	const FString& ImageBase64,
	const FString& Prompt,
	const FLLMRequestConfig& Config)
{
	// Build JSON for multimodal request
	// Format varies by provider (Anthropic vs OpenAI)

	if (Config.Provider == ELLMProvider::Anthropic)
	{
		// Anthropic format
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		JsonObject->SetStringField(TEXT("model"), Config.ModelName);
		JsonObject->SetNumberField(TEXT("max_tokens"), Config.MaxTokens);

		// Messages array
		TArray<TSharedPtr<FJsonValue>> Messages;

		TSharedPtr<FJsonObject> UserMsg = MakeShareable(new FJsonObject);
		UserMsg->SetStringField(TEXT("role"), TEXT("user"));

		// Content array with image and text
		TArray<TSharedPtr<FJsonValue>> Content;

		// Image part
		TSharedPtr<FJsonObject> ImagePart = MakeShareable(new FJsonObject);
		ImagePart->SetStringField(TEXT("type"), TEXT("image"));
		TSharedPtr<FJsonObject> ImageSource = MakeShareable(new FJsonObject);
		ImageSource->SetStringField(TEXT("type"), TEXT("base64"));
		ImageSource->SetStringField(TEXT("media_type"), TEXT("image/png"));
		ImageSource->SetStringField(TEXT("data"), ImageBase64);
		ImagePart->SetObjectField(TEXT("source"), ImageSource);
		Content.Add(MakeShareable(new FJsonValueObject(ImagePart)));

		// Text part
		TSharedPtr<FJsonObject> TextPart = MakeShareable(new FJsonObject);
		TextPart->SetStringField(TEXT("type"), TEXT("text"));
		TextPart->SetStringField(TEXT("text"), Prompt);
		Content.Add(MakeShareable(new FJsonValueObject(TextPart)));

		UserMsg->SetArrayField(TEXT("content"), Content);
		Messages.Add(MakeShareable(new FJsonValueObject(UserMsg)));

		JsonObject->SetArrayField(TEXT("messages"), Messages);

		FString OutputString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
		FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

		return OutputString;
	}
	else if (Config.Provider == ELLMProvider::OpenAI)
	{
		// OpenAI format (GPT-4 Vision)
		// Similar but slightly different JSON structure
		// TODO: Implement OpenAI vision format
		return FString();
	}

	return FString();
}

FVisualAnalysisResult UVisionSystem::ParseVisualResponse(const FString& Response)
{
	FVisualAnalysisResult Result;
	Result.Description = Response;

	// Parse the response to extract structured information
	// Look for YES/NO
	if (Response.Contains(TEXT("YES"), ESearchCase::IgnoreCase) && !Response.Contains(TEXT("NO"), ESearchCase::IgnoreCase))
	{
		Result.bMatchesRequirements = true;
	}

	// Extract quality score if present (0-10 scale)
	FString ScorePattern = TEXT("score");
	int32 ScoreIndex = Response.Find(ScorePattern, ESearchCase::IgnoreCase);
	if (ScoreIndex != INDEX_NONE)
	{
		// Try to find a number after "score"
		FString AfterScore = Response.Mid(ScoreIndex + ScorePattern.Len());
		FString NumberStr;
		for (int32 i = 0; i < AfterScore.Len(); i++)
		{
			TCHAR C = AfterScore[i];
			if (FChar::IsDigit(C) || C == '.')
			{
				NumberStr += C;
			}
			else if (!NumberStr.IsEmpty())
			{
				break;
			}
		}

		if (!NumberStr.IsEmpty())
		{
			float Score = FCString::Atof(*NumberStr);
			Result.QualityScore = FMath::Clamp(Score / 10.0f, 0.0f, 1.0f); // Normalize to 0-1
		}
	}

	// TODO: More sophisticated parsing
	// - Extract detected objects
	// - Extract issues
	// - Extract suggestions

	return Result;
}

void UVisionSystem::OnAnalysisResponseReceived(const FString& Response)
{
	FVisualAnalysisResult Result = ParseVisualResponse(Response);
	CurrentSuccessCallback.ExecuteIfBound(Result);
}

void UVisionSystem::OnAnalysisFailed(const FString& Error)
{
	CurrentFailureCallback.ExecuteIfBound(Error);
}

// ============================================================================
// VISUAL VERIFICATION SYSTEM
// ============================================================================

void UVisualVerificationSystem::VerifyWorkAutomatically(
	const FString& TaskDescription,
	const FString& Requirements,
	const FLLMRequestConfig& Config,
	FOnVisualAnalysisComplete OnComplete,
	FOnVisionFailed OnFailed)
{
	if (!VisionSystem)
	{
		VisionSystem = NewObject<UVisionSystem>(this);
	}

	// Capture screenshot
	FScreenshotSettings Settings;
	FString ScreenshotPath = UVisionSystem::CaptureViewportScreenshot(Settings);

	if (ScreenshotPath.IsEmpty())
	{
		OnFailed.ExecuteIfBound(TEXT("Failed to capture screenshot"));
		return;
	}

	// Verify
	VisionSystem->VerifyAgainstRequirements(Requirements, ScreenshotPath, Config, OnComplete, OnFailed);
}

void UVisualVerificationSystem::VerifyFromMultipleAngles(
	const FVector& TargetLocation,
	const TArray<FRotator>& CameraAngles,
	const FString& Requirements,
	const FLLMRequestConfig& Config,
	FOnVisualAnalysisComplete OnComplete,
	FOnVisionFailed OnFailed)
{
	// TODO: Implement multi-angle verification
	// Capture from each angle, analyze each, combine results

	if (!VisionSystem)
	{
		VisionSystem = NewObject<UVisionSystem>(this);
	}

	// For now, just use the first angle
	if (CameraAngles.Num() > 0)
	{
		FScreenshotSettings Settings;
		FString ScreenshotPath = UVisionSystem::CaptureFromPosition(TargetLocation, CameraAngles[0], Settings);

		if (ScreenshotPath.IsEmpty())
		{
			OnFailed.ExecuteIfBound(TEXT("Failed to capture screenshot"));
			return;
		}

		VisionSystem->VerifyAgainstRequirements(Requirements, ScreenshotPath, Config, OnComplete, OnFailed);
	}
}

void UVisualVerificationSystem::VerifyActor(
	AActor* Actor,
	const FString& Requirements,
	const FLLMRequestConfig& Config,
	FOnVisualAnalysisComplete OnComplete,
	FOnVisionFailed OnFailed)
{
	if (!VisionSystem)
	{
		VisionSystem = NewObject<UVisionSystem>(this);
	}

	if (!Actor)
	{
		OnFailed.ExecuteIfBound(TEXT("No actor provided"));
		return;
	}

	// Capture actor screenshot
	FScreenshotSettings Settings;
	FString ScreenshotPath = UVisionSystem::CaptureActorScreenshot(Actor, Settings);

	if (ScreenshotPath.IsEmpty())
	{
		OnFailed.ExecuteIfBound(TEXT("Failed to capture actor screenshot"));
		return;
	}

	// Verify
	VisionSystem->VerifyAgainstRequirements(Requirements, ScreenshotPath, Config, OnComplete, OnFailed);
}
