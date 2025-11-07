// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

/**
 * User session information
 */
struct FCollaborativeSession
{
	FString SessionID;
	FString UserName;
	FLinearColor UserColor;        // Color for this user's selection
	TArray<AActor*> SelectedActors;  // Current selection
	FVector CameraLocation;
	FRotator CameraRotation;
	double LastActivityTime;
	bool bIsActive;
};

/**
 * Edit lock for actors
 */
struct FActorLock
{
	AActor* LockedActor;
	FString OwnerSessionID;
	FString OwnerUserName;
	double LockTime;
};

/**
 * Collaborative editing message
 */
struct FCollaborativeMessage
{
	enum class EMessageType
	{
		UserJoined,
		UserLeft,
		SelectionChanged,
		ActorModified,
		ActorLocked,
		ActorUnlocked,
		ChatMessage
	};

	EMessageType Type;
	FString SessionID;
	FString UserName;
	FString Message;
	TArray<AActor*> AffectedActors;
	double Timestamp;
};

/**
 * Collaborative multi-user editing system
 * TIER 4.2 Feature - Future Vision
 */
class CLAUDETERMINALEDITOR_API FCollaborativeSystem
{
public:
	/**
	 * Initialize collaborative system
	 * @param UserName Name of local user
	 * @return true if initialized
	 */
	static bool Initialize(const FString& UserName);

	/**
	 * Shutdown collaborative system
	 */
	static void Shutdown();

	/**
	 * Join collaborative session
	 * @param SessionID Session to join
	 * @return true if joined successfully
	 */
	static bool JoinSession(const FString& SessionID);

	/**
	 * Leave current session
	 */
	static void LeaveSession();

	/**
	 * Get active users in session
	 * @param OutSessions Active user sessions
	 * @return Number of active users
	 */
	static int32 GetActiveUsers(TArray<FCollaborativeSession>& OutSessions);

	/**
	 * Lock actor for editing
	 * @param Actor Actor to lock
	 * @return true if lock acquired
	 */
	static bool LockActor(AActor* Actor);

	/**
	 * Unlock actor
	 * @param Actor Actor to unlock
	 * @return true if unlocked
	 */
	static bool UnlockActor(AActor* Actor);

	/**
	 * Check if actor is locked by another user
	 * @param Actor Actor to check
	 * @param OutOwnerName Owner's name if locked
	 * @return true if locked
	 */
	static bool IsActorLocked(AActor* Actor, FString& OutOwnerName);

	/**
	 * Broadcast selection change
	 * @param SelectedActors Current selection
	 */
	static void BroadcastSelection(const TArray<AActor*>& SelectedActors);

	/**
	 * Broadcast actor modification
	 * @param ModifiedActor Actor that was modified
	 */
	static void BroadcastActorModification(AActor* ModifiedActor);

	/**
	 * Send chat message
	 * @param Message Message text
	 */
	static void SendChatMessage(const FString& Message);

	/**
	 * Get recent messages
	 * @param OutMessages Recent messages
	 * @param Count Number of messages to retrieve
	 * @return Number of messages
	 */
	static int32 GetRecentMessages(TArray<FCollaborativeMessage>& OutMessages, int32 Count = 10);

	/**
	 * Update camera position (for showing to other users)
	 * @param Location Camera location
	 * @param Rotation Camera rotation
	 */
	static void UpdateCameraPosition(const FVector& Location, const FRotator& Rotation);

	/**
	 * Check if collaborative mode is active
	 */
	static bool IsActive();

	/**
	 * Get local session
	 */
	static FCollaborativeSession* GetLocalSession();

private:
	// Session management
	static TArray<FCollaborativeSession> ActiveSessions;
	static FCollaborativeSession LocalSession;
	static TArray<FActorLock> ActorLocks;
	static TArray<FCollaborativeMessage> MessageHistory;
	static bool bIsInitialized;

	// Network sync (placeholder - would use actual networking)
	static void SyncToNetwork();
	static void ReceiveFromNetwork();

	// Conflict resolution
	static void ResolveConflict(AActor* Actor);

	// Visual indicators
	static void UpdateUserIndicators(UWorld* World);
};
