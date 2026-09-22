#include "PSMatchGameState.h"

#include "Net/UnrealNetwork.h"

void APSMatchGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APSMatchGameState, MatchState);
}

float APSMatchGameState::GetPhaseSecondsRemaining() const
{
    return FMath::Max(0.0, MatchState.PhaseEndServerTime - GetServerWorldTimeSeconds());
}

float APSMatchGameState::GetDefuseStageSecondsRemaining() const
{
    return FMath::Max(0.0, MatchState.DefuseStageEndServerTime - GetServerWorldTimeSeconds());
}

void APSMatchGameState::NotifyChanged()
{
    OnChanged.Broadcast();
    ForceNetUpdate();
}

void APSMatchGameState::OnRep_MatchState()
{
    OnChanged.Broadcast();
}
