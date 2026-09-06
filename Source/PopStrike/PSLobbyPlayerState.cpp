#include "PSLobbyPlayerState.h"

#include "Net/UnrealNetwork.h"
#include "PSLobbyGameState.h"

void APSLobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ThisClass, Team);
    DOREPLIFETIME(ThisClass, JoinOrder);
    DOREPLIFETIME(ThisClass, AvatarBytes);
}

void APSLobbyPlayerState::SetTeam(EPSLobbyTeam Value)
{
    if (HasAuthority() && Team != Value)
    {
        Team = Value;
        ForceNetUpdate();
        NotifyLobbyChanged();
    }
}

void APSLobbyPlayerState::SetJoinOrder(int32 Value)
{
    if (HasAuthority() && JoinOrder != Value)
    {
        JoinOrder = Value;
        ForceNetUpdate();
        NotifyLobbyChanged();
    }
}

void APSLobbyPlayerState::SetProfile(const FString& Name, const TArray<uint8>& Bytes)
{
    if (!HasAuthority())
    {
        return;
    }

    SetPlayerName(Name);
    AvatarBytes = Bytes;
    ForceNetUpdate();
    NotifyLobbyChanged();
}

void APSLobbyPlayerState::OnRep_PlayerName()
{
    Super::OnRep_PlayerName();
    NotifyLobbyChanged();
}

void APSLobbyPlayerState::OnRep_LobbyState()
{
    NotifyLobbyChanged();
}

void APSLobbyPlayerState::OnRep_Avatar()
{
    NotifyLobbyChanged();
}

void APSLobbyPlayerState::NotifyLobbyChanged() const
{
    if (UWorld* World = GetWorld())
    {
        if (APSLobbyGameState* GS = World->GetGameState<APSLobbyGameState>())
        {
            GS->NotifyLobbyChanged();
        }
    }
}
