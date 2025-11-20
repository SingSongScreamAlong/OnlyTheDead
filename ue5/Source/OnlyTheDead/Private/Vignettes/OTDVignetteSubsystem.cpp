//// BEGIN NEW CODE
#include "Vignettes/OTDVignetteSubsystem.h"

#include "Components/OTDMentalStateComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "OTDLogging.h"

namespace
{
    constexpr float EPSILON = 0.01f;
}

void UOTDVignetteSubsystem::RecordAnalytics(const FOTDActiveVignette& Vignette, bool bCompleted)
{
    FOTDVignetteAnalyticsRecord Record;
    Record.InstanceId = Vignette.InstanceId;
    Record.EventTag = Vignette.Definition.EventTag;
    Record.StartTimestamp = Vignette.StartTimestamp;
    Record.EndTimestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : Vignette.StartTimestamp;
    Record.bCompleted = bCompleted;
    AnalyticsRecords.Add(Record);
}

void UOTDVignetteSubsystem::ResetAnalyticsRecords()
{
    AnalyticsRecords.Reset();
}

UOTDVignetteSubsystem::UOTDVignetteSubsystem()
{
    ContextStack.Reserve(8);
    Definitions.Reserve(32);
    ActiveVignettes.Reserve(8);
}

void UOTDVignetteSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UOTDVignetteSubsystem::Deinitialize()
{
    ContextStack.Empty();
    Definitions.Empty();
    ActiveVignettes.Empty();
    CooldownMap.Empty();
    Super::Deinitialize();
}

void UOTDVignetteSubsystem::Tick(float DeltaTime)
{
    UpdateActiveVignettes(DeltaTime);
    EvaluateVignetteSpawns(DeltaTime);
}

void UOTDVignetteSubsystem::PushContext(const FOTDVignetteContext& Context)
{
    ContextStack.Add(Context);
}

void UOTDVignetteSubsystem::PopContext()
{
    if (ContextStack.Num() > 0)
    {
        ContextStack.Pop();
    }
}

void UOTDVignetteSubsystem::RegisterVignetteDefinition(const FOTDVignetteEventDefinition& Definition)
{
    Definitions.Add(Definition);
}

void UOTDVignetteSubsystem::ClearDefinitions()
{
    Definitions.Empty();
}

void UOTDVignetteSubsystem::UpdateActiveVignettes(float DeltaTime)
{
    for (int32 Index = ActiveVignettes.Num() - 1; Index >= 0; --Index)
    {
        FOTDActiveVignette& Vignette = ActiveVignettes[Index];
        if (Vignette.State == EOTDVignetteState::Active)
        {
            Vignette.TimeRemaining -= DeltaTime;
            if (Vignette.TimeRemaining <= 0.f)
            {
                Vignette.State = EOTDVignetteState::Completed;
                if (Vignette.Definition.CompletionSound)
                {
                    UGameplayStatics::PlaySoundAtLocation(GetWorld(), Vignette.Definition.CompletionSound, Vignette.Origin);
                }
                RecordAnalytics(Vignette, true);
                OnVignetteCompleted.Broadcast(Vignette.InstanceId, Vignette.Definition);
                ActiveVignettes.RemoveAt(Index);
            }
        }
    }
}

void UOTDVignetteSubsystem::EvaluateVignetteSpawns(float DeltaTime)
{
    if (Definitions.Num() == 0)
    {
        return;
    }

    if (ActiveVignettes.Num() >= MaxConcurrentVignettes)
    {
        return;
    }

    if (!GetWorld())
    {
        return;
    }

    const double Now = GetWorld()->GetTimeSeconds();
    if (Now - LastVignetteStartTime < MinIntervalBetweenStarts)
    {
        return;
    }

    float TotalWeight = 0.f;
    TArray<int32> EligibleIndices;

    for (int32 Index = 0; Index < Definitions.Num(); ++Index)
    {
        const FOTDVignetteEventDefinition& Definition = Definitions[Index];
        if (!CanStartVignette(Definition))
        {
            continue;
        }

        const float Weight = ComputeVignetteWeight(Definition);
        if (Weight <= EPSILON)
        {
            continue;
        }

        EligibleIndices.Add(Index);
        TotalWeight += Weight;
    }

    if (EligibleIndices.Num() == 0 || TotalWeight <= EPSILON)
    {
        return;
    }

    const float SelectionValue = FMath::FRandRange(0.f, TotalWeight);
    float AccumWeight = 0.f;

    for (int32 Index : EligibleIndices)
    {
        const float Weight = ComputeVignetteWeight(Definitions[Index]);
        AccumWeight += Weight;
        if (SelectionValue <= AccumWeight)
        {
            StartVignette(Definitions[Index]);
            break;
        }
    }
}

bool UOTDVignetteSubsystem::CanStartVignette(const FOTDVignetteEventDefinition& Definition) const
{
    const double* LastTime = CooldownMap.Find(Definition.EventTag);
    const double Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
    if (LastTime)
    {
        const double Cooldown = FMath::FRandRange(Definition.MinCooldown, Definition.MaxCooldown);
        if (Now - *LastTime < Cooldown)
        {
            return false;
        }
    }

    for (const FOTDActiveVignette& Vignette : ActiveVignettes)
    {
        if (Vignette.Definition.EventTag == Definition.EventTag && Vignette.State == EOTDVignetteState::Active)
        {
            return false;
        }
    }

    return true;
}

float UOTDVignetteSubsystem::ComputeVignetteWeight(const FOTDVignetteEventDefinition& Definition) const
{
    float Weight = Definition.BaseWeight;
    if (ContextStack.Num() == 0)
    {
        return Weight;
    }

    const FOTDVignetteContext& Context = ContextStack.Last();

    if (!Context.Tags.HasAll(Definition.RequiredContextTags))
    {
        return 0.f;
    }

    if (Context.Tags.HasAny(Definition.ExcludedContextTags))
    {
        return 0.f;
    }

    Weight *= 1.f + Context.Urgency;
    return Weight;
}

void UOTDVignetteSubsystem::StartVignette(const FOTDVignetteEventDefinition& Definition)
{
    FOTDActiveVignette Vignette;
    Vignette.Definition = Definition;
    Vignette.InstanceId = FGuid::NewGuid();
    Vignette.State = EOTDVignetteState::Active;
    Vignette.TimeRemaining = Definition.DurationSeconds;
    Vignette.StartTimestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
    if (const FOTDVignetteContext* Context = GetActiveContext())
    {
        Vignette.Origin = Context->FocusLocation;
        Vignette.InfluenceRadius = Context->Radius;
    }

    ActiveVignettes.Add(Vignette);
    CooldownMap.Add(Definition.EventTag, Vignette.StartTimestamp);
    LastVignetteStartTime = Vignette.StartTimestamp;

    if (Definition.StartSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), Definition.StartSound, Vignette.Origin);
    }

    if (Definition.SuppressionPulse > 0.f)
    {
        ApplySuppressionPulse(Vignette.Origin, Definition.SuppressionRadius, Definition.SuppressionPulse);
    }

    if (bDebugLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Vignette started: %s"), *Definition.EventTag.ToString());
    }

    OnVignetteStarted.Broadcast(Vignette.InstanceId, Definition);
}

const FOTDVignetteContext* UOTDVignetteSubsystem::GetActiveContext() const
{
    return ContextStack.Num() > 0 ? &ContextStack.Last() : nullptr;
}

void UOTDVignetteSubsystem::ApplySuppressionPulse(const FVector& Origin, float Radius, float Intensity) const
{
    if (!GetWorld())
    {
        return;
    }

    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(VignetteSuppression), false);

    if (!GetWorld()->OverlapMultiByChannel(Overlaps, Origin, FQuat::Identity, ECC_Pawn, Sphere, Params))
    {
        return;
    }

    for (const FOverlapResult& Result : Overlaps)
    {
        if (AActor* Actor = Result.GetActor())
        {
            if (UOTDMentalStateComponent* Mental = Actor->FindComponentByClass<UOTDMentalStateComponent>())
            {
                Mental->RegisterSuppressionImpulse(Intensity);
            }
        }
    }
}
//// END NEW CODE
