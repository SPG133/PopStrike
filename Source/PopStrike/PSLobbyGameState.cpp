#include "PSLobbyGameState.h"

#include "Net/UnrealNetwork.h"
#include "PSGameConfig.h"
#include "PSLobbyPlayerState.h"

void APSLobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ThisClass, SelectedMapIndex);
}

void APSLobbyGameState::AddPlayerState(APlayerState* PlayerState)
{
    Super::AddPlayerState(PlayerState);
    NotifyLobbyChanged();
}

void APSLobbyGameState::RemovePlayerState(APlayerState* PlayerState)
{
    Super::RemovePlayerState(PlayerState);
    NotifyLobbyChanged();
}

APSLobbyPlayerState* APSLobbyGameState::GetHost() const
{
    APSLobbyPlayerState* Host = nullptr;
    for (APlayerState* Base : PlayerArray)
    {
        APSLobbyPlayerState* PS = Cast<APSLobbyPlayerState>(Base);
        if (PS && PS->GetJoinOrder() != INDEX_NONE &&
            (!Host || PS->GetJoinOrder() < Host->GetJoinOrder()))
        {
            Host = PS;
        }
    }
    return Host;
}

int32 APSLobbyGameState::CountTeam(EPSLobbyTeam Team) const
{
    int32 Count = 0;
    for (APlayerState* Base : PlayerArray)
    {
        const APSLobbyPlayerState* PS = Cast<APSLobbyPlayerState>(Base);
        Count += PS && PS->GetTeam() == Team ? 1 : 0;
    }
    return Count;
}

bool APSLobbyGameState::AreTeamsReady() const
{
    return CountTeam(EPSLobbyTeam::TeamA) == PSConfig::TeamSize &&
           CountTeam(EPSLobbyTeam::TeamB) == PSConfig::TeamSize;
}

void APSLobbyGameState::GetTeamPlayers(EPSLobbyTeam Team, TArray<APSLobbyPlayerState*>& OutPlayers) const
{
    OutPlayers.Reset();
    for (APlayerState* Base : PlayerArray)
    {
        APSLobbyPlayerState* PS = Cast<APSLobbyPlayerState>(Base);
        if (PS && PS->GetTeam() == Team)
        {
            OutPlayers.Add(PS);
        }
    }

    OutPlayers.Sort([](const APSLobbyPlayerState& A, const APSLobbyPlayerState& B)
    {
        return A.GetJoinOrder() < B.GetJoinOrder();
    });
}

void APSLobbyGameState::SetSelectedMapIndex(int32 Index)
{
    if (!HasAuthority())
    {
        return;
    }

    const int32 Wrapped = PSLobby::WrapMapIndex(Index);
    if (Wrapped != INDEX_NONE && Wrapped != SelectedMapIndex)
    {
        SelectedMapIndex = Wrapped;
        ForceNetUpdate();
        NotifyLobbyChanged();
    }
}

void APSLobbyGameState::OnRep_SelectedMapIndex()
{
    NotifyLobbyChanged();
}

void APSLobbyGameState::NotifyLobbyChanged()
{
    OnLobbyChanged.Broadcast();
}
