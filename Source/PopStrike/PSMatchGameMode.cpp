#include "PSMatchGameMode.h"

#include "PSMatchGameState.h"
#include "PSMatchPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

namespace
{
    EPSLobbyTeam OtherTeam(EPSLobbyTeam Team)
    {
        if (Team == EPSLobbyTeam::A) return EPSLobbyTeam::B;
        if (Team == EPSLobbyTeam::B) return EPSLobbyTeam::A;
        return EPSLobbyTeam::None;
    }

    bool IsPlayableTeam(EPSLobbyTeam Team)
    {
        return Team == EPSLobbyTeam::A || Team == EPSLobbyTeam::B;
    }

    float SafeDuration(float Duration)
    {
        return FMath::Max(Duration, UE_KINDA_SMALL_NUMBER);
    }
}

APSMatchGameMode::APSMatchGameMode()
{
    GameStateClass = APSMatchGameState::StaticClass();
    PlayerStateClass = APSMatchPlayerState::StaticClass();
}

void APSMatchGameMode::InitGame(
    const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    ExpectedPlayers = FMath::Clamp(
        UGameplayStatics::GetIntOption(Options, TEXT("ExpectedPlayers"), 1), 1, 6);
}

void APSMatchGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
    Super::HandleStartingNewPlayer_Implementation(NewPlayer);
    TryStartMatch();
}

void APSMatchGameMode::Logout(AController* Exiting)
{
    if (ActiveDefuser.Get() == Exiting)
    {
        ClearDefuse();
    }

    Super::Logout(Exiting);
}

EPSLobbyTeam APSMatchGameMode::GetTeam(const AController* Player) const
{
    const auto* State = Player ? Player->GetPlayerState<APSMatchPlayerState>() : nullptr;
    return State ? State->Team : EPSLobbyTeam::None;
}

void APSMatchGameMode::TryStartMatch()
{
    auto* State = GetGameState<APSMatchGameState>();
    if (!State || State->MatchState.Phase != EPSMatchPhase::Waiting ||
        State->PlayerArray.Num() < ExpectedPlayers) return;

    for (const APlayerState* Player : State->PlayerArray)
    {
        const auto* MatchPlayer = Cast<APSMatchPlayerState>(Player);
        if (!MatchPlayer || !IsPlayableTeam(MatchPlayer->Team)) return;
    }

    State->MatchState.RoundNumber = 1;
    EnterFreeze();
}

void APSMatchGameMode::SetPhase(EPSMatchPhase Phase, float Duration)
{
    auto* State = GetGameState<APSMatchGameState>();
    if (!State) return;

    State->MatchState.Phase = Phase;
    State->MatchState.PhaseEndServerTime =
        Duration > 0.f ? State->GetServerWorldTimeSeconds() + SafeDuration(Duration) : 0.0;
    State->NotifyChanged();
}

void APSMatchGameMode::EnterFreeze()
{
    auto* State = GetGameState<APSMatchGameState>();
    if (!State) return;

    GetWorldTimerManager().ClearTimer(PhaseTimer);
    ClearDefuse(false);
    State->MatchState.DefuseStage = 0;
    const bool bDecidingRound =
        State->MatchState.ScoreA == WinScore - 1 &&
        State->MatchState.ScoreB == WinScore - 1;
    const float CurrentFreezeDuration = bDecidingRound
        ? DecidingRoundFreezeDuration
        : FreezeDuration;
    SetPhase(EPSMatchPhase::Freeze, CurrentFreezeDuration);
    GetWorldTimerManager().SetTimer(
        PhaseTimer, this, &ThisClass::EnterLive, SafeDuration(CurrentFreezeDuration), false);
}

void APSMatchGameMode::EnterLive()
{
    SetPhase(EPSMatchPhase::Live, PlantDuration);
    GetWorldTimerManager().SetTimer(
        PhaseTimer, this, &ThisClass::RoundTimeout, SafeDuration(PlantDuration), false);
}

void APSMatchGameMode::RoundTimeout()
{
    const auto* State = GetGameState<APSMatchGameState>();
    if (!State || State->MatchState.Phase != EPSMatchPhase::Live) return;
    EndRound(OtherTeam(State->MatchState.AttackingTeam));
}

void APSMatchGameMode::PlantBomb(AController* Planter)
{
    auto* State = GetGameState<APSMatchGameState>();
    if (!State || State->MatchState.Phase != EPSMatchPhase::Live ||
        GetTeam(Planter) != State->MatchState.AttackingTeam) return;

    GetWorldTimerManager().ClearTimer(PhaseTimer);
    ClearDefuse(false);
    SetPhase(EPSMatchPhase::BombPlanted, BombDuration);
    GetWorldTimerManager().SetTimer(
        PhaseTimer, this, &ThisClass::BombExploded, SafeDuration(BombDuration), false);
}

void APSMatchGameMode::BombExploded()
{
    const auto* State = GetGameState<APSMatchGameState>();
    if (!State || State->MatchState.Phase != EPSMatchPhase::BombPlanted) return;
    EndRound(State->MatchState.AttackingTeam);
}

void APSMatchGameMode::StartDefuse(AController* Defuser)
{
    auto* State = GetGameState<APSMatchGameState>();
    if (!State || State->MatchState.Phase != EPSMatchPhase::BombPlanted ||
        GetTeam(Defuser) != OtherTeam(State->MatchState.AttackingTeam) ||
        ActiveDefuser.IsValid()) return;

    ActiveDefuser = Defuser;
    const float StageDuration = SafeDuration(DefuseDuration * 0.5f);
    State->MatchState.DefuseStageEndServerTime =
        State->GetServerWorldTimeSeconds() + StageDuration;
    State->NotifyChanged();
    GetWorldTimerManager().SetTimer(
        DefuseTimer, this, &ThisClass::FinishDefuseStage, StageDuration, false);
}

void APSMatchGameMode::CancelDefuse(AController* Defuser)
{
    if (ActiveDefuser.Get() != Defuser) return;
    ClearDefuse();
}

void APSMatchGameMode::FinishDefuseStage()
{
    auto* State = GetGameState<APSMatchGameState>();
    if (!State || !ActiveDefuser.IsValid() ||
        State->MatchState.Phase != EPSMatchPhase::BombPlanted) return;

    if (State->MatchState.DefuseStage == 0)
    {
        State->MatchState.DefuseStage = 1;
        const float StageDuration = SafeDuration(DefuseDuration * 0.5f);
        State->MatchState.DefuseStageEndServerTime =
            State->GetServerWorldTimeSeconds() + StageDuration;
        State->NotifyChanged();
        GetWorldTimerManager().SetTimer(
            DefuseTimer, this, &ThisClass::FinishDefuseStage, StageDuration, false);
        return;
    }

    EndRound(OtherTeam(State->MatchState.AttackingTeam));
}

void APSMatchGameMode::EndRound(EPSLobbyTeam Winner)
{
    auto* State = GetGameState<APSMatchGameState>();
    if (!State || (State->MatchState.Phase != EPSMatchPhase::Live &&
        State->MatchState.Phase != EPSMatchPhase::BombPlanted) ||
        !IsPlayableTeam(Winner)) return;

    GetWorldTimerManager().ClearTimer(PhaseTimer);
    ClearDefuse(false);
    ++(Winner == EPSLobbyTeam::A ? State->MatchState.ScoreA : State->MatchState.ScoreB);
    State->MatchState.LastRoundWinner = Winner;
    SetPhase(EPSMatchPhase::RoundEnd, RoundEndDuration);
    GetWorldTimerManager().SetTimer(
        PhaseTimer, this, &ThisClass::AdvanceRound, SafeDuration(RoundEndDuration), false);
}

void APSMatchGameMode::AdvanceRound()
{
    auto* State = GetGameState<APSMatchGameState>();
    if (!State || State->MatchState.Phase != EPSMatchPhase::RoundEnd) return;

    if (State->MatchState.ScoreA >= WinScore || State->MatchState.ScoreB >= WinScore)
    {
        State->MatchState.WinnerTeam =
            State->MatchState.ScoreA >= WinScore ? EPSLobbyTeam::A : EPSLobbyTeam::B;
        SetPhase(EPSMatchPhase::MatchEnd, 0.f);
        return;
    }

    const bool bEnteringDecidingRound =
        State->MatchState.ScoreA == WinScore - 1 &&
        State->MatchState.ScoreB == WinScore - 1;
    if (bEnteringDecidingRound)
    {
        State->MatchState.AttackingTeam =
            FMath::RandBool() ? EPSLobbyTeam::A : EPSLobbyTeam::B;
    }
    else if (State->MatchState.RoundNumber == 7)
    {
        State->MatchState.AttackingTeam = OtherTeam(State->MatchState.AttackingTeam);
    }

    ++State->MatchState.RoundNumber;
    EnterFreeze();
}

void APSMatchGameMode::ClearDefuse(bool bNotifyClients)
{
    GetWorldTimerManager().ClearTimer(DefuseTimer);
    ActiveDefuser.Reset();

    if (auto* State = GetGameState<APSMatchGameState>())
    {
        if (State->MatchState.DefuseStageEndServerTime != 0.0)
        {
            State->MatchState.DefuseStageEndServerTime = 0.0;
            if (bNotifyClients) State->NotifyChanged();
        }
    }
}
