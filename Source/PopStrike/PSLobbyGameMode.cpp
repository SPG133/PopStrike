#include "PSLobbyGameMode.h"

#include "PSLobbyGameState.h"
#include "PSLobbyPlayerController.h"
#include "PSLobbyPlayerState.h"

APSLobbyGameMode::APSLobbyGameMode()
{
    PlayerControllerClass = APSLobbyPlayerController::StaticClass();
    PlayerStateClass = APSLobbyPlayerState::StaticClass();
    GameStateClass = APSLobbyGameState::StaticClass();
    DefaultPawnClass = nullptr;
    bStartPlayersAsSpectators = true;
    bUseSeamlessTravel = true;

    const auto AddMap = [this](const TCHAR* Name, const TCHAR* Description,
        const TCHAR* LevelPath, const TCHAR* PreviewPath)
    {
        FPSLobbyMap Map;
        Map.Name = FText::FromString(Name);
        Map.Description = FText::FromString(Description);
        Map.Level = TSoftObjectPtr<UWorld>(FSoftObjectPath(LevelPath));
        Map.Preview = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(PreviewPath));
        AvailableMaps.Add(MoveTemp(Map));
    };

    AddMap(
        TEXT("果汁工厂"),
        TEXT("在果汁生产线中对战。"),
        TEXT("/Game/Maps/Level_3/Juice_Factory/L_JuiceFactory.L_JuiceFactory"),
        TEXT("/Game/UI/MapPreviews/T_JuiceFactoryPreview.T_JuiceFactoryPreview"));
}

void APSLobbyGameMode::InitGameState()
{
    Super::InitGameState();
    GetGameState<APSLobbyGameState>()->Maps = AvailableMaps;
}

void APSLobbyGameMode::PreLogin(const FString &Options, const FString &Address,
                                const FUniqueNetIdRepl &UniqueId, FString &Error)
{
    Super::PreLogin(Options, Address, UniqueId, Error);
    const auto *Lobby = GetGameState<APSLobbyGameState>();
    if (Error.IsEmpty() && Lobby && (Lobby->bStarting || Lobby->PlayerArray.Num() >= 6))
        Error = TEXT("Room is full or starting.");
}

void APSLobbyGameMode::PostLogin(APlayerController *Player)
{
    Super::PostLogin(Player);
    auto *Lobby = GetGameState<APSLobbyGameState>();
    if (!Lobby->Host)
    {
        auto *PS = Player->GetPlayerState<APSLobbyPlayerState>();

        Lobby->Host = PS;

        if (PS)
        {
            PS->bIsHost = true;
            PS->ForceNetUpdate();
        }
    }
    // 进房自动随机分队；目标队满 3 人则去另一队。玩家之后仍可在客厅自由换队
    if (auto* PS = Player->GetPlayerState<APSLobbyPlayerState>(); PS && PS->Team == EPSLobbyTeam::None)
    {
        EPSLobbyTeam Team = FMath::RandBool() ? EPSLobbyTeam::A : EPSLobbyTeam::B;
        if (Lobby->GetTeamPlayers(Team).Num() >= 3)
            Team = (Team == EPSLobbyTeam::A) ? EPSLobbyTeam::B : EPSLobbyTeam::A;
        PS->Team = Team;
        PS->ForceNetUpdate();
    }

    Lobby->BroadcastChanged();
    CastChecked<APSLobbyPlayerController>(Player)->ClientRequestProfile();
}

void APSLobbyGameMode::Logout(AController *Player)
{
    auto *Lobby = GetGameState<APSLobbyGameState>();
    const auto *Leaving = Player->GetPlayerState<APSLobbyPlayerState>();
    if (Lobby->Host == Leaving)
    {

        Lobby->Host = nullptr;

        for (APlayerState *Other : Lobby->PlayerArray)
        {
            if (Other == Leaving)
            {
                continue;
            }

            if (auto *NewHost = Cast<APSLobbyPlayerState>(Other))
            {
                Lobby->Host = NewHost;

                NewHost->bIsHost = true;
                NewHost->ForceNetUpdate();

                break;
            }
        }
    }

    Super::Logout(Player);
    Lobby->BroadcastChanged();
}
