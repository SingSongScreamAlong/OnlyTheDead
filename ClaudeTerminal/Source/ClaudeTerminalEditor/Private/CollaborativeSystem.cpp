// Copyright Epic Games, Inc. All Rights Reserved.

#include "CollaborativeSystem.h"

// Static member initialization
TArray<FCollaborativeSession> FCollaborativeSystem::ActiveSessions;
FCollaborativeSession FCollaborativeSystem::LocalSession;
TArray<FActorLock> FCollaborativeSystem::ActorLocks;
TArray<FCollaborativeMessage> FCollaborativeSystem::MessageHistory;
bool FCollaborativeSystem::bIsInitialized = false;

bool FCollaborativeSystem::Initialize(const FString& UserName)
{
	UE_LOG(LogTemp, Log, TEXT("Collaborative: Initializing for user '%s'"), *UserName);

	LocalSession.SessionID = FGuid::NewGuid().ToString();
	LocalSession.UserName = UserName;
	LocalSession.UserColor = FLinearColor::MakeRandomColor();
	LocalSession.LastActivityTime = FPlatformTime::Seconds();
	LocalSession.bIsActive = true;

	ActiveSessions.Add(LocalSession);

	bIsInitialized = true;
	return true;
}

void FCollaborativeSystem::Shutdown()
{
	if (!bIsInitialized)
		return;

	UE_LOG(LogTemp, Log, TEXT("Collaborative: Shutting down"));

	LeaveSession();
	ActiveSessions.Empty();
	ActorLocks.Empty();
	MessageHistory.Empty();

	bIsInitialized = false;
}

bool FCollaborativeSystem::JoinSession(const FString& SessionID)
{
	if (!bIsInitialized)
		return false;

	UE_LOG(LogTemp, Log, TEXT("Collaborative: Joining session '%s'"), *SessionID);

	// Broadcast join message
	FCollaborativeMessage Message;
	Message.Type = FCollaborativeMessage::EMessageType::UserJoined;
	Message.SessionID = LocalSession.SessionID;
	Message.UserName = LocalSession.UserName;
	Message.Message = FString::Printf(TEXT("%s joined the session"), *LocalSession.UserName);
	Message.Timestamp = FPlatformTime::Seconds();

	MessageHistory.Add(Message);

	return true;
}

void FCollaborativeSystem::LeaveSession()
{
	if (!bIsInitialized)
		return;

	// Unlock all actors owned by this user
	for (int32 i = ActorLocks.Num() - 1; i >= 0; i--)
	{
		if (ActorLocks[i].OwnerSessionID == LocalSession.SessionID)
		{
			ActorLocks.RemoveAt(i);
		}
	}

	// Broadcast leave message
	FCollaborativeMessage Message;
	Message.Type = FCollaborativeMessage::EMessageType::UserLeft;
	Message.SessionID = LocalSession.SessionID;
	Message.UserName = LocalSession.UserName;
	Message.Message = FString::Printf(TEXT("%s left the session"), *LocalSession.UserName);
	Message.Timestamp = FPlatformTime::Seconds();

	MessageHistory.Add(Message);

	LocalSession.bIsActive = false;
}

int32 FCollaborativeSystem::GetActiveUsers(TArray<FCollaborativeSession>& OutSessions)
{
	OutSessions = ActiveSessions.FilterByPredicate([](const FCollaborativeSession& Session) {
		return Session.bIsActive;
	});

	return OutSessions.Num();
}

bool FCollaborativeSystem::LockActor(AActor* Actor)
{
	if (!Actor || !bIsInitialized)
		return false;

	// Check if already locked
	FString OwnerName;
	if (IsActorLocked(Actor, OwnerName))
	{
		UE_LOG(LogTemp, Warning, TEXT("Collaborative: Actor already locked by %s"), *OwnerName);
		return false;
	}

	// Create lock
	FActorLock Lock;
	Lock.LockedActor = Actor;
	Lock.OwnerSessionID = LocalSession.SessionID;
	Lock.OwnerUserName = LocalSession.UserName;
	Lock.LockTime = FPlatformTime::Seconds();

	ActorLocks.Add(Lock);

	UE_LOG(LogTemp, Log, TEXT("Collaborative: Actor locked by %s"), *LocalSession.UserName);

	return true;
}

bool FCollaborativeSystem::UnlockActor(AActor* Actor)
{
	if (!Actor || !bIsInitialized)
		return false;

	for (int32 i = 0; i < ActorLocks.Num(); i++)
	{
		if (ActorLocks[i].LockedActor == Actor &&
			ActorLocks[i].OwnerSessionID == LocalSession.SessionID)
		{
			ActorLocks.RemoveAt(i);
			UE_LOG(LogTemp, Log, TEXT("Collaborative: Actor unlocked"));
			return true;
		}
	}

	return false;
}

bool FCollaborativeSystem::IsActorLocked(AActor* Actor, FString& OutOwnerName)
{
	if (!Actor)
		return false;

	for (const FActorLock& Lock : ActorLocks)
	{
		if (Lock.LockedActor == Actor)
		{
			OutOwnerName = Lock.OwnerUserName;
			return true;
		}
	}

	return false;
}

void FCollaborativeSystem::BroadcastSelection(const TArray<AActor*>& SelectedActors)
{
	if (!bIsInitialized)
		return;

	LocalSession.SelectedActors = SelectedActors;

	FCollaborativeMessage Message;
	Message.Type = FCollaborativeMessage::EMessageType::SelectionChanged;
	Message.SessionID = LocalSession.SessionID;
	Message.UserName = LocalSession.UserName;
	Message.AffectedActors = SelectedActors;
	Message.Timestamp = FPlatformTime::Seconds();

	MessageHistory.Add(Message);
}

void FCollaborativeSystem::BroadcastActorModification(AActor* ModifiedActor)
{
	if (!ModifiedActor || !bIsInitialized)
		return;

	FCollaborativeMessage Message;
	Message.Type = FCollaborativeMessage::EMessageType::ActorModified;
	Message.SessionID = LocalSession.SessionID;
	Message.UserName = LocalSession.UserName;
	Message.AffectedActors.Add(ModifiedActor);
	Message.Message = FString::Printf(TEXT("%s modified %s"),
		*LocalSession.UserName, *ModifiedActor->GetName());
	Message.Timestamp = FPlatformTime::Seconds();

	MessageHistory.Add(Message);
}

void FCollaborativeSystem::SendChatMessage(const FString& Message)
{
	if (!bIsInitialized)
		return;

	FCollaborativeMessage ChatMsg;
	ChatMsg.Type = FCollaborativeMessage::EMessageType::ChatMessage;
	ChatMsg.SessionID = LocalSession.SessionID;
	ChatMsg.UserName = LocalSession.UserName;
	ChatMsg.Message = Message;
	ChatMsg.Timestamp = FPlatformTime::Seconds();

	MessageHistory.Add(ChatMsg);

	UE_LOG(LogTemp, Log, TEXT("Collaborative: [%s]: %s"), *LocalSession.UserName, *Message);
}

int32 FCollaborativeSystem::GetRecentMessages(TArray<FCollaborativeMessage>& OutMessages, int32 Count)
{
	OutMessages.Empty();

	int32 StartIndex = FMath::Max(0, MessageHistory.Num() - Count);

	for (int32 i = StartIndex; i < MessageHistory.Num(); i++)
	{
		OutMessages.Add(MessageHistory[i]);
	}

	return OutMessages.Num();
}

void FCollaborativeSystem::UpdateCameraPosition(const FVector& Location, const FRotator& Rotation)
{
	if (!bIsInitialized)
		return;

	LocalSession.CameraLocation = Location;
	LocalSession.CameraRotation = Rotation;
	LocalSession.LastActivityTime = FPlatformTime::Seconds();
}

bool FCollaborativeSystem::IsActive()
{
	return bIsInitialized && LocalSession.bIsActive;
}

FCollaborativeSession* FCollaborativeSystem::GetLocalSession()
{
	return bIsInitialized ? &LocalSession : nullptr;
}

void FCollaborativeSystem::SyncToNetwork()
{
	// Placeholder for network synchronization
	// Would send LocalSession state and messages to server/peers
}

void FCollaborativeSystem::ReceiveFromNetwork()
{
	// Placeholder for receiving network updates
	// Would update ActiveSessions, ActorLocks, MessageHistory
}

void FCollaborativeSystem::ResolveConflict(AActor* Actor)
{
	// Conflict resolution strategy
	// - Last-write-wins
	// - Operational transformation
	// - Manual merge
}

void FCollaborativeSystem::UpdateUserIndicators(UWorld* World)
{
	// Draw visual indicators for other users:
	// - Selection outlines in user colors
	// - Camera position markers
	// - User name labels
}

/*
 * COLLABORATIVE WORKFLOW EXAMPLE:
 *
 * Designer A: "I'm working on the trench system"
 * - LockActor(TrenchActors)
 * - Others see red outline on trenches
 *
 * Designer B: "I'll add buildings in village area"
 * - LockActor(VillageArea)
 * - Can see Designer A's camera position
 *
 * System: Prevents conflicts
 * - Designer B tries to edit trench → "Locked by Designer A"
 * - Chat messages: "Designer A: How many buildings do you need?"
 *
 * Full implementation would include:
 * - WebSocket or UDP networking
 * - Operational transformation for concurrent edits
 * - Visual indicators (colored outlines, nameplates)
 * - Voice chat integration
 */
