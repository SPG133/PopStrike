#pragma once

#include "CoreMinimal.h"
#include "PSLobbyTypes.h"
#include "PSMatchTypes.generated.h"

UENUM(BlueprintType)
enum class EPSMatchPhase : uint8
{
    Waiting,
    Freeze,
    Live,
    BombPlanted,
    RoundEnd,
    MatchEnd
};

USTRUCT(BlueprintType)
struct FPSMatchState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) EPSMatchPhase Phase = EPSMatchPhase::Waiting;
    UPROPERTY(BlueprintReadOnly) int32 RoundNumber = 0;
    UPROPERTY(BlueprintReadOnly) int32 ScoreA = 0;
    UPROPERTY(BlueprintReadOnly) int32 ScoreB = 0;
    UPROPERTY(BlueprintReadOnly) EPSLobbyTeam AttackingTeam = EPSLobbyTeam::A;
    UPROPERTY(BlueprintReadOnly) EPSLobbyTeam LastRoundWinner = EPSLobbyTeam::None;
    UPROPERTY(BlueprintReadOnly) EPSLobbyTeam WinnerTeam = EPSLobbyTeam::None;
    UPROPERTY(BlueprintReadOnly) double PhaseEndServerTime = 0.0;
    UPROPERTY(BlueprintReadOnly) uint8 DefuseStage = 0;
    UPROPERTY(BlueprintReadOnly) double DefuseStageEndServerTime = 0.0;
};
