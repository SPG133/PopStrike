#include "PSLobbyPlayerController.h"

#include "PSAvatar.h"
#include "PSConnectionSubsystem.h"
#include "PSLobbyGameState.h"
#include "PSLobbyPlayerState.h"
#include "Engine/GameInstance.h"
#include "Misc/PackageName.h"

bool APSLobbyPlayerController::IsRoomHost() const
{
    const auto* Lobby = GetWorld()->GetGameState<APSLobbyGameState>();
    return Lobby && Lobby->Host == PlayerState;
}

void APSLobbyPlayerController::ClientRequestProfile_Implementation()
{
    const auto* Profile = GetGameInstance()->GetSubsystem<UPSConnectionSubsystem>();
    ServerSubmitProfile(Profile->GetPlayerName(), Profile->AvatarBytes);
}

void APSLobbyPlayerController::ServerSubmitProfile_Implementation(
    const FString& Name, const TArray<uint8>& Bytes)
{
    if (bProfileSubmitted) return;
    bProfileSubmitted = true;

    auto* LobbyPlayer = GetPlayerState<APSLobbyPlayerState>();
    if (!LobbyPlayer) return;

    const FString CleanName = Name.TrimStartAndEnd();
    const bool bBadName = CleanName.IsEmpty() || CleanName.Len() > 16 ||
        CleanName.Contains(TEXT("\n")) || CleanName.Contains(TEXT("\r"));
    LobbyPlayer->SetPlayerName(bBadName ? TEXT("Player") : CleanName);
    if (PSAvatar::IsValid(Bytes)) LobbyPlayer->AvatarBytes = Bytes;
    else ClientLobbyError(FText::FromString(TEXT("头像数据无效。")));

    LobbyPlayer->ForceNetUpdate();
    if (auto* Lobby = GetWorld()->GetGameState<APSLobbyGameState>()) Lobby->BroadcastChanged();
}

void APSLobbyPlayerController::ServerJoinTeam_Implementation(EPSLobbyTeam Team)
{
    auto* Lobby = GetWorld()->GetGameState<APSLobbyGameState>();
    auto* LobbyPlayer = GetPlayerState<APSLobbyPlayerState>();
    if (!Lobby || !LobbyPlayer || Lobby->bStarting ||
        (Team != EPSLobbyTeam::A && Team != EPSLobbyTeam::B)) return;
    if (LobbyPlayer->Team == Team) return;
    if (Lobby->GetTeamPlayers(Team).Num() >= 3)
    {
        ClientLobbyError(FText::FromString(TEXT("该队伍已满。")));
        return;
    }

    LobbyPlayer->Team = Team;
    LobbyPlayer->ForceNetUpdate();
    Lobby->BroadcastChanged();
}

void APSLobbyPlayerController::ServerSelectMap_Implementation(int32 MapIndex)
{
    auto* Lobby = GetWorld()->GetGameState<APSLobbyGameState>();
    if (!Lobby || !IsRoomHost() || Lobby->bStarting || !Lobby->Maps.IsValidIndex(MapIndex)) return;
    Lobby->SelectedMap = MapIndex;
    Lobby->BroadcastChanged();
}

void APSLobbyPlayerController::ServerStartGame_Implementation()
{
    auto* Lobby = GetWorld()->GetGameState<APSLobbyGameState>();
    if (!Lobby || !IsRoomHost() || Lobby->bStarting || !Lobby->Maps.IsValidIndex(Lobby->SelectedMap)) return;

    const FString Map = Lobby->Maps[Lobby->SelectedMap].Level.ToSoftObjectPath().GetLongPackageName();
    if (!FPackageName::DoesPackageExist(Map))
    {
        ClientLobbyError(FText::FromString(TEXT("地图不存在或未打包。")));
        return;
    }

    Lobby->bStarting = true;
    Lobby->BroadcastChanged();
    GetWorld()->ServerTravel(Map);
}

void APSLobbyPlayerController::ClientLobbyError_Implementation(const FText& Message)
{
    OnLobbyError.Broadcast(Message);
}
