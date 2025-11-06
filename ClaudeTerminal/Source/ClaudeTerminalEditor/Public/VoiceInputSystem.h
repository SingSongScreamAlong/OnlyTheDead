// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_DELEGATE_OneParam(FOnVoiceTranscribed, const FString&);
DECLARE_DELEGATE_OneParam(FOnVoiceError, const FString&);

/**
 * Voice input state
 */
UENUM()
enum class EVoiceInputState : uint8
{
	Idle,
	Listening,
	Processing,
	Error
};

/**
 * Voice input system for speech-to-text commands
 * TIER 2.6 Feature
 *
 * Supports platform-specific speech recognition:
 * - Windows: Windows Speech Recognition
 * - Mac: macOS Speech Recognition
 * - Cross-platform: WebRTC-based fallback
 */
class CLAUDETERMINALEDITOR_API FVoiceInputSystem
{
public:
	FVoiceInputSystem();
	~FVoiceInputSystem();

	/**
	 * Initialize voice input system
	 * @return true if initialization successful
	 */
	bool Initialize();

	/**
	 * Shutdown voice input system
	 */
	void Shutdown();

	/**
	 * Start listening for voice input (push-to-talk)
	 * @param OnTranscribed Callback when speech is transcribed
	 * @param OnError Callback on error
	 * @return true if listening started
	 */
	bool StartListening(FOnVoiceTranscribed OnTranscribed, FOnVoiceError OnError);

	/**
	 * Stop listening
	 */
	void StopListening();

	/**
	 * Check if currently listening
	 */
	bool IsListening() const { return CurrentState == EVoiceInputState::Listening; }

	/**
	 * Get current state
	 */
	EVoiceInputState GetState() const { return CurrentState; }

	/**
	 * Check if voice input is available on this platform
	 */
	static bool IsVoiceInputAvailable();

	/**
	 * Get supported languages
	 */
	static TArray<FString> GetSupportedLanguages();

	/**
	 * Set recognition language
	 * @param LanguageCode Language code (e.g., "en-US", "fr-FR")
	 */
	void SetLanguage(const FString& LanguageCode);

	/**
	 * Get current recognition language
	 */
	FString GetCurrentLanguage() const { return CurrentLanguage; }

	/**
	 * Enable/disable noise cancellation
	 */
	void SetNoiseCancellation(bool bEnabled);

	/**
	 * Get last transcription confidence (0.0-1.0)
	 */
	float GetLastConfidence() const { return LastConfidence; }

private:
	// Platform-specific initialization
	bool InitializePlatformVoice();

	// Platform-specific shutdown
	void ShutdownPlatformVoice();

	// Platform-specific start listening
	bool StartPlatformListening();

	// Platform-specific stop listening
	void StopPlatformListening();

	// Transcription callback from platform
	void OnPlatformTranscription(const FString& Transcription, float Confidence);

	// Error callback from platform
	void OnPlatformError(const FString& ErrorMessage);

	// Current state
	EVoiceInputState CurrentState;

	// Callbacks
	FOnVoiceTranscribed OnTranscribedCallback;
	FOnVoiceError OnErrorCallback;

	// Settings
	FString CurrentLanguage;
	bool bNoiseCancellationEnabled;

	// Last result
	float LastConfidence;

	// Platform-specific handles/objects
	void* PlatformHandle;

	// Initialization flag
	bool bInitialized;
};
