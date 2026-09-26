#include "PSMatchGameMode.h"

#include "PSMatchGameState.h"
#include "PSMatchPlayerState.h"
#include "PSFPSCharacter.h"
#include "PSMatchPlayerController.h"
#include "Engine/PlayerStartPIE.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
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

    FName StartTagForTeam(EPSLobbyTeam Team)
    {
        if (Team == EPSLobbyTeam::A) return TEXT("TeamA");
        if (Team == EPSLobbyTeam::B) return TEXT("TeamB");
        return NAME_None;
    }
}

APSMatchGameMode::APSMatchGameMode()
{
    GameStateClass = APSMatchGameState::StaticClass();
    PlayerStateClass = APSMatchPlayerState::StaticClass();
    DefaultPawnClass = APSFPSCharacter::StaticClass();
    PlayerControllerClass = APSMatchPlayerController::StaticClass();
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
    // Seamless travel already copied the lobby team. Only assign direct entrants.
    if (auto* Player = NewPlayer->GetPlayerState<APSMatchPlayerState>(); Player && Player->Team == EPSLobbyTeam::None)
    {
        int32 TeamA = 0, TeamB = 0;
        for (const TObjectPtr<APlayerState>& Other : GameState->PlayerArray)
        {
            if (const auto* MatchPlayer = Cast<APSMatchPlayerState>(Other.Get()))
            {
                TeamA += MatchPlayer->Team == EPSLobbyTeam::A;
                TeamB += MatchPlayer->Team == EPSLobbyTeam::B;
            }
        }
        Player->Team = TeamA <= TeamB ? EPSLobbyTeam::A : EPSLobbyTeam::B;
    }
    Super::HandleStartingNewPlayer_Implementation(NewPlayer);
    TryStartMatch();
}

AActor* APSMatchGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    const FName RequiredTag = StartTagForTeam(GetTeam(Player));
    if (RequiredTag.IsNone()) return Super::ChoosePlayerStart_Implementation(Player);

    UWorld* World = GetWorld();
    UClass* PawnClass = GetDefaultPawnClassForController(Player);
    APawn* PawnToFit = PawnClass ? PawnClass->GetDefaultObject<APawn>() : nullptr;
    TArray<APlayerStart*> UnoccupiedStarts;
    TArray<APlayerStart*> OccupiedStarts;

    for (TActorIterator<APlayerStart> It(World); It; ++It)
    {
        APlayerStart* Start = *It;
        if (Start->IsA<APlayerStartPIE>()) return Start;
        if (Start->PlayerStartTag != RequiredTag) continue;

        FVector Location = Start->GetActorLocation();
        const FRotator Rotation = Start->GetActorRotation();
        if (!World->EncroachingBlockingGeometry(PawnToFit, Location, Rotation))
        {
            UnoccupiedStarts.Add(Start);
        }
        else if (World->FindTeleportSpot(PawnToFit, Location, Rotation))
        {
            OccupiedStarts.Add(Start);
        }
    }

    if (!UnoccupiedStarts.IsEmpty())
        return UnoccupiedStarts[FMath::RandHelper(UnoccupiedStarts.Num())];
    if (!OccupiedStarts.IsEmpty())
        return OccupiedStarts[FMath::RandHelper(OccupiedStarts.Num())];

    return Super::ChoosePlayerStart_Implementation(Player);
}

void APSMatchGameMode::PreLogin(const FString& Options, const FString& Address,
    const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
    if (ErrorMessage.IsEmpty() && GameState && GameState->PlayerArray.Num() >= 6)
        ErrorMessage = TEXT("Match is full.");
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
    // A fresh Pawn gives every round a separate lifetime for input and shot sequences.
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* Player = It->Get();
        if (!Player) continue;
        if (APawn* OldPawn = Player->GetPawn())
        {
            if (State->MatchState.RoundNumber == 1) continue;
            Player->UnPossess();
            OldPawn->Destroy();
        }
        RestartPlayer(Player);
    }
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
