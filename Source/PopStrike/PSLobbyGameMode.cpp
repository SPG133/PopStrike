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

    FPSLobbyMap Map;
    Map.Name = FText::FromString(TEXT("果汁工厂"));
    Map.Level = TSoftObjectPtr<UWorld>(FSoftObjectPath(
        TEXT("/Game/Maps/Level_3/Juice_Factory/L_JuiceFactory.L_JuiceFactory")));
    AvailableMaps.Add(Map);
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
