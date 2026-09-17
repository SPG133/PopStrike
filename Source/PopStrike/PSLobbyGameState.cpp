#include "PSLobbyGameState.h"

#include "PSLobbyPlayerState.h"
#include "Net/UnrealNetwork.h"

void APSLobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APSLobbyGameState, Maps);
    DOREPLIFETIME(APSLobbyGameState, SelectedMap);
    DOREPLIFETIME(APSLobbyGameState, Host);
    DOREPLIFETIME(APSLobbyGameState, bStarting);
}

TArray<APSLobbyPlayerState*> APSLobbyGameState::GetTeamPlayers(EPSLobbyTeam Team) const
{
    TArray<APSLobbyPlayerState*> Result;
    for (APlayerState* Player : PlayerArray)
        if (auto* LobbyPlayer = Cast<APSLobbyPlayerState>(Player); LobbyPlayer && LobbyPlayer->Team == Team)
            Result.Add(LobbyPlayer);
    return Result;
}

void APSLobbyGameState::OnRep_Data()
{
    OnChanged.Broadcast();
}

void APSLobbyGameState::BroadcastChanged()
{
    OnChanged.Broadcast();
    if (HasAuthority()) ForceNetUpdate();
}

void APSLobbyGameState::AddPlayerState(APlayerState* PlayerState)
{
    Super::AddPlayerState(PlayerState);
    BroadcastChanged();
}

void APSLobbyGameState::RemovePlayerState(APlayerState* PlayerState)
{
    Super::RemovePlayerState(PlayerState);
    BroadcastChanged();
}
