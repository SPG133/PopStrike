#include "PSLobbyPlayerState.h"

#include "PSLobbyGameState.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "Net/UnrealNetwork.h"

void APSLobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APSLobbyPlayerState, Team);
    DOREPLIFETIME(APSLobbyPlayerState, AvatarBytes);
}

UTexture2D* APSLobbyPlayerState::GetAvatar()
{
    if (!AvatarTexture && !AvatarBytes.IsEmpty() && GetNetMode() != NM_DedicatedServer)
        AvatarTexture = FImageUtils::ImportBufferAsTexture2D(AvatarBytes);
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
