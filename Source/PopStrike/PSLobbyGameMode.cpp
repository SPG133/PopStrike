#include "PSLobbyGameMode.h"

#include "Misc/PackageName.h"
#include "PSGameConfig.h"
#include "PSLobbyGameState.h"
#include "PSLobbyPlayerController.h"
#include "PSLobbyPlayerState.h"
#include "Engine/Engine.h"
#include "GameFramework/GameSession.h"

APSLobbyGameMode::APSLobbyGameMode()
{
    PlayerControllerClass = APSLobbyPlayerController::StaticClass();
    PlayerStateClass = APSLobbyPlayerState::StaticClass();
    GameStateClass = APSLobbyGameState::StaticClass();
    DefaultPawnClass = nullptr;
    HUDClass = nullptr;
}

void APSLobbyGameMode::PreLogin(
    const FString& Options,
    const FString& Address,
    const FUniqueNetIdRepl& UniqueId,
    FString& ErrorMessage)
{
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
    if (ErrorMessage.IsEmpty() && bTraveling) ErrorMessage = TEXT("Match is starting. Please retry later.");
    if (ErrorMessage.IsEmpty() && GetNumPlayers() >= PSConfig::MaxPlayers)
    {
        ErrorMessage = TEXT("房间已满（6/6）。");
    }
}

void APSLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    // PreLogin checks can overlap; enforce capacity again after actual admission.
    if (bTraveling || GetNumPlayers() > PSConfig::MaxPlayers)
    {
        if (GameSession) GameSession->KickPlayer(NewPlayer, FText::FromString(TEXT("Room full or match starting.")));
        return;
    }
    if (APSLobbyPlayerState* PS = NewPlayer ? NewPlayer->GetPlayerState<APSLobbyPlayerState>() : nullptr)
    {
        PS->SetJoinOrder(NextJoinOrder++);
    }
}

void APSLobbyGameMode::BeginPlay()
{
    Super::BeginPlay();
    if (GEngine) GEngine->OnTravelFailure().AddUObject(this, &ThisClass::HandleTravelFailure);
}

void APSLobbyGameMode::EndPlay(const EEndPlayReason::Type Reason)
{
    if (GEngine) GEngine->OnTravelFailure().RemoveAll(this);
    Super::EndPlay(Reason);
}

void APSLobbyGameMode::HandleTravelFailure(UWorld* World, ETravelFailure::Type Type, const FString& Error)
{
    if (World != GetWorld()) return;
    bTraveling = false;
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
        SendMessage(Cast<APSLobbyPlayerController>(It->Get()), TEXT("Map travel failed. Please retry."));
}

bool APSLobbyGameMode::IsHost(const APSLobbyPlayerController* PC) const
{
    const APSLobbyGameState* GS = GetGameState<APSLobbyGameState>();
    return GS && PC && PC->PlayerState == GS->GetHost();
}

void APSLobbyGameMode::SendMessage(APSLobbyPlayerController* PC, const FString& Message) const
{
    if (PC)
    {
        PC->ClientLobbyMessage(FText::FromString(Message));
    }
}

void APSLobbyGameMode::TrySetTeam(APSLobbyPlayerController* PC, EPSLobbyTeam Team)
{
    if (!PC || bTraveling || (Team != EPSLobbyTeam::TeamA && Team != EPSLobbyTeam::TeamB))
    {
        return;
    }

    APSLobbyPlayerState* PS = PC->GetPlayerState<APSLobbyPlayerState>();
    APSLobbyGameState* GS = GetGameState<APSLobbyGameState>();
    if (!PS || !GS || PS->GetTeam() == Team)
    {
        return;
    }

    if (GS->CountTeam(Team) >= PSConfig::TeamSize)
    {
        SendMessage(PC, TEXT("这个阵营已经满员。"));
        return;
    }

    PS->SetTeam(Team);
}

void APSLobbyGameMode::TryChangeMap(APSLobbyPlayerController* PC, int32 Direction)
{
    APSLobbyGameState* GS = GetGameState<APSLobbyGameState>();
    if (!GS || !IsHost(PC) || bTraveling || Direction == 0)
    {
        return;
    }

    GS->SetSelectedMapIndex(GS->GetSelectedMapIndex() + (Direction < 0 ? -1 : 1));
}

void APSLobbyGameMode::TryStartMatch(APSLobbyPlayerController* PC)
{
    APSLobbyGameState* GS = GetGameState<APSLobbyGameState>();
    if (!GS || !IsHost(PC) || bTraveling)
    {
        return;
    }

    if (!GS->AreTeamsReady())
    {
        SendMessage(PC, TEXT("需要双方各 3 人才能开始。"));
        return;
    }

    const FPSLobbyMapDefinition* Map = PSLobby::GetMap(GS->GetSelectedMapIndex());
    if (!Map || !FPackageName::DoesPackageExist(Map->LevelPackage))
    {
        SendMessage(PC, TEXT("所选地图不存在，请检查 /Game/Maps。"));
        return;
    }

    bTraveling = true;
    if (!GetWorld()->ServerTravel(Map->LevelPackage))
    {
        bTraveling = false;
        SendMessage(PC, TEXT("Map travel failed. Please retry."));
    }
}
