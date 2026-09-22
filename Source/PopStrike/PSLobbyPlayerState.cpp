#include "PSLobbyPlayerState.h"

#include "PSAvatar.h"
#include "PSLobbyGameState.h"
#include "PSMatchPlayerState.h"
#include "Net/UnrealNetwork.h"

void APSLobbyPlayerState::CopyProperties(APlayerState* PlayerState)
{
    Super::CopyProperties(PlayerState);
    if (auto* MatchPlayer = Cast<APSMatchPlayerState>(PlayerState))
    {
        MatchPlayer->Team = Team;
        MatchPlayer->AvatarBytes = AvatarBytes;
    }
}

void APSLobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APSLobbyPlayerState, Team);
    DOREPLIFETIME(APSLobbyPlayerState, bIsHost);
    DOREPLIFETIME(APSLobbyPlayerState, AvatarBytes);
}

UTexture2D* APSLobbyPlayerState::GetAvatar()
{
    if (!AvatarTexture && GetNetMode() != NM_DedicatedServer)
    {
        AvatarTexture = PSAvatar::CreateTexture(AvatarBytes, GetPlayerId());
    }
    return AvatarTexture;
}

void APSLobbyPlayerState::OnRep_Data()
{
    if (auto* Lobby = GetWorld()->GetGameState<APSLobbyGameState>()) Lobby->BroadcastChanged();
}

void APSLobbyPlayerState::OnRep_Avatar()
{
    AvatarTexture = nullptr;
    OnRep_Data();
}

void APSLobbyPlayerState::OnRep_PlayerName()
{
    Super::OnRep_PlayerName();
    OnRep_Data();
}
