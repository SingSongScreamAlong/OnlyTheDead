#include "GameMode/VerdunGameMode.h"
#include "Artillery/ArtilleryManager.h"
#include "Player/VerdunSoldier.h"

#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AVerdunGameMode::AVerdunGameMode()
{
    DefaultPawnClass = AVerdunSoldier::StaticClass();
}

void AVerdunGameMode::BeginPlay()
{
    Super::BeginPlay();

    FindArtilleryManager();

    // Give the player a moment to orient themselves before shells arrive
    if (ArtilleryManager && BarrageStartDelaySeconds > 0.0f)
    {
        // Disable auto-start so we control timing here
        ArtilleryManager->bAutoStartOnBeginPlay = false;

        GetWorldTimerManager().SetTimer(
            BarrageStartTimer,
            this,
            &AVerdunGameMode::StartBarrage,
            BarrageStartDelaySeconds,
            false
        );

        UE_LOG(LogTemp, Log, TEXT("VerdunGameMode: Barrage starts in %.0f seconds."), BarrageStartDelaySeconds);
    }
}

void AVerdunGameMode::FindArtilleryManager()
{
    TArray<AActor*> Managers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AArtilleryManager::StaticClass(), Managers);

    if (Managers.Num() > 0)
    {
        ArtilleryManager = Cast<AArtilleryManager>(Managers[0]);
        UE_LOG(LogTemp, Log, TEXT("VerdunGameMode: Found ArtilleryManager."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("VerdunGameMode: No ArtilleryManager found in level! Add one and set ShellClass."));
    }
}

void AVerdunGameMode::StartBarrage()
{
    if (ArtilleryManager)
    {
        ArtilleryManager->StartBarrage();
    }
}

void AVerdunGameMode::StopBarrage()
{
    if (ArtilleryManager)
    {
        ArtilleryManager->StopBarrage();
    }
}
