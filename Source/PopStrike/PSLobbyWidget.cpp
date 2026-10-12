#include "PSLobbyWidget.h"

#include "PSLobbyGameState.h"
#include "PSLobbyPlayerController.h"
#include "PSLobbyPlayerState.h"

void UPSLobbyWidget::NativeConstruct()
{
    Super::NativeConstruct();
    LobbyState = GetWorld()->GetGameState<APSLobbyGameState>();
    LobbyController = Cast<APSLobbyPlayerController>(GetOwningPlayer());
    if (LobbyState)
        LobbyState->OnChanged.AddUniqueDynamic(this, &ThisClass::RefreshLobby);
    if (LobbyController)
        LobbyController->OnLobbyError.AddUniqueDynamic(this, &ThisClass::HandleLobbyError);
    RefreshLobby();
}

void UPSLobbyWidget::NativeDestruct()
{
    if (LobbyState)
        LobbyState->OnChanged.RemoveDynamic(this, &ThisClass::RefreshLobby);
    if (LobbyController)
        LobbyController->OnLobbyError.RemoveDynamic(this, &ThisClass::HandleLobbyError);
    Super::NativeDestruct();
}

void UPSLobbyWidget::RefreshLobby()
{
    TeamA = LobbyState ? LobbyState->GetTeamPlayers(EPSLobbyTeam::A) : TArray<APSLobbyPlayerState *>();
    TeamB = LobbyState ? LobbyState->GetTeamPlayers(EPSLobbyTeam::B) : TArray<APSLobbyPlayerState *>();
    bHasMap = LobbyState && LobbyState->Maps.IsValidIndex(LobbyState->SelectedMap);
    if (bHasMap)
        CurrentMap = LobbyState->Maps[LobbyState->SelectedMap];
    bIsHost = LobbyController && LobbyController->IsRoomHost();
    bCanInteract = LobbyState && !LobbyState->bStarting;
    BP_OnLobbyRefreshed();
}

void UPSLobbyWidget::ChangeMap(int32 Offset)
{
    if (!LobbyController || !LobbyState || LobbyState->Maps.IsEmpty())
        return;
    const int32 Count = LobbyState->Maps.Num();
    LobbyController->ServerSelectMap((LobbyState->SelectedMap + Offset % Count + Count) % Count);
}

void UPSLobbyWidget::HandleLobbyError(const FText &Message)
{
    BP_OnLobbyError(Message);
}
