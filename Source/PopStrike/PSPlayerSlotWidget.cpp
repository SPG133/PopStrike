#include "PSPlayerSlotWidget.h"

#include "PSLobbyGameState.h"
#include "PSLobbyPlayerState.h"

void UPSPlayerSlotWidget::SetPlayer(APSLobbyPlayerState* InPlayer)
{
    Player = InPlayer;
    PlayerName = Player ? FText::FromString(Player->GetPlayerName()) : FText::FromString(TEXT("等待玩家"));
    Avatar = Player ? Player->GetAvatar() : nullptr;
    Team = Player ? Player->Team : EPSLobbyTeam::None;
    const auto* Lobby = GetWorld()->GetGameState<APSLobbyGameState>();
    bIsHost = Player && Lobby && Lobby->Host == Player;
    BP_OnPlayerChanged();
}
