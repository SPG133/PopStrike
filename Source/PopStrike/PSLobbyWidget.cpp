#include "PSLobbyWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "PSGameConfig.h"
#include "PSLobbyGameState.h"
#include "PSLobbyPlayerController.h"
#include "PSLobbyPlayerState.h"
#include "PSLobbyTypes.h"
#include "PSPlayerSlotWidget.h"
#include "TimerManager.h"

void UPSLobbyWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);


    BindGameState();
    Refresh();
    if (GetWorld()) GetWorld()->GetTimerManager().SetTimer(StateTimer, this, &ThisClass::Refresh, 0.5f, true);
}

void UPSLobbyWidget::NativeDestruct()
{
    if (BoundGameState)
    {
        BoundGameState->OnLobbyChanged.RemoveAll(this);
    }
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(MessageTimer);
        GetWorld()->GetTimerManager().ClearTimer(StateTimer);
    }
    Super::NativeDestruct();
}

void UPSLobbyWidget::BindGameState()
{
    if (BoundGameState) BoundGameState->OnLobbyChanged.RemoveAll(this);
    BoundGameState = GetWorld() ? GetWorld()->GetGameState<APSLobbyGameState>() : nullptr;
    if (BoundGameState)
    {
        BoundGameState->OnLobbyChanged.AddUObject(this, &ThisClass::Refresh);
    }
}

void UPSLobbyWidget::ShowServerMessage(const FText& Message)
{
    bHoldingServerMessage = true;
    Txt_RoomStatusHint->SetText(Message);
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(MessageTimer, this, &ThisClass::ClearServerMessage, 2.0f, false);
    }
}

void UPSLobbyWidget::ClearServerMessage()
{
    bHoldingServerMessage = false;
    Refresh();
}

void UPSLobbyWidget::Refresh()
{
    if (GetWorld() && BoundGameState != GetWorld()->GetGameState<APSLobbyGameState>()) BindGameState();
    APSLobbyGameState* GS = BoundGameState ? BoundGameState.Get() :
        (GetWorld() ? GetWorld()->GetGameState<APSLobbyGameState>() : nullptr);
    APSLobbyPlayerController* PC = GetLobbyController();
    if (!GS || !PC)
    {
        return;
    }

    RefreshPlayers(GS);
    RefreshControls(GS, PC);
}

void UPSLobbyWidget::RefreshPlayers(APSLobbyGameState* GS)
{
    UPSPlayerSlotWidget* ASlots[] = {Slot_A1, Slot_A2, Slot_A3};
    UPSPlayerSlotWidget* BSlots[] = {Slot_B1, Slot_B2, Slot_B3};

    TArray<APSLobbyPlayerState*> APlayers;
    TArray<APSLobbyPlayerState*> BPlayers;
    GS->GetTeamPlayers(EPSLobbyTeam::TeamA, APlayers);
    GS->GetTeamPlayers(EPSLobbyTeam::TeamB, BPlayers);
    for (int32 I = APlayers.Num(); I < int32(UE_ARRAY_COUNT(ASlots)); ++I) ASlots[I]->SetEmpty();
    for (int32 I = BPlayers.Num(); I < int32(UE_ARRAY_COUNT(BSlots)); ++I) BSlots[I]->SetEmpty();

    APSLobbyPlayerState* Host = GS->GetHost();
    for (int32 I = 0; I < FMath::Min(APlayers.Num(), int32(UE_ARRAY_COUNT(ASlots))); ++I)
    {
        APSLobbyPlayerState* PS = APlayers[I];
        ASlots[I]->SetPlayer(PS->GetPlayerName(), EPSLobbyTeam::TeamA, PS == Host, PS->GetAvatarBytes());
    }
    for (int32 I = 0; I < FMath::Min(BPlayers.Num(), int32(UE_ARRAY_COUNT(BSlots))); ++I)
    {
        APSLobbyPlayerState* PS = BPlayers[I];
        BSlots[I]->SetPlayer(PS->GetPlayerName(), EPSLobbyTeam::TeamB, PS == Host, PS->GetAvatarBytes());
    }
}

void UPSLobbyWidget::RefreshControls(APSLobbyGameState* GS, APSLobbyPlayerController* PC)
{
    APSLobbyPlayerState* LocalPS = PC->GetPlayerState<APSLobbyPlayerState>();
    APSLobbyPlayerState* Host = GS->GetHost();
    const bool bLocalHost = LocalPS && LocalPS == Host;
    const int32 TeamA = GS->CountTeam(EPSLobbyTeam::TeamA);
    const int32 TeamB = GS->CountTeam(EPSLobbyTeam::TeamB);

    Btn_JoinTeamA->SetIsEnabled(LocalPS && LocalPS->GetTeam() != EPSLobbyTeam::TeamA && TeamA < PSConfig::TeamSize);
    Btn_JoinTeamB->SetIsEnabled(LocalPS && LocalPS->GetTeam() != EPSLobbyTeam::TeamB && TeamB < PSConfig::TeamSize);
    Btn_StartGame->SetVisibility(bLocalHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    Btn_StartGame->SetIsEnabled(bLocalHost && GS->AreTeamsReady());

    if (!bHoldingServerMessage)
    {
        const FString Status = GS->AreTeamsReady()
            ? (bLocalHost ? TEXT("双方已满，可以拉响警报。") : TEXT("双方已满，等待房主开战……"))
            : FString::Printf(TEXT("A %d/3  B %d/3，等待玩家……"), TeamA, TeamB);
        Txt_RoomStatusHint->SetText(FText::FromString(Status));
    }

    RefreshMap(GS, bLocalHost);
}

void UPSLobbyWidget::RefreshMap(APSLobbyGameState* GS, bool bLocalHost)
{
    const int32 MapIndex = GS->GetSelectedMapIndex();
    const FPSLobbyMapDefinition* Map = PSLobby::GetMap(MapIndex);
    if (!Map)
    {
        return;
    }

    Txt_MapName->SetText(FText::FromString(Map->Name));
    Txt_MapDescription->SetText(FText::FromString(Map->Description));
    Txt_MapIndex->SetText(FText::FromString(FString::Printf(
        TEXT("%d / %d"), MapIndex + 1, PSLobby::GetMaps().Num())));
    Txt_MapHostOnly->SetText(FText::FromString(
        bLocalHost ? TEXT("你是房主，可以切换地图") : TEXT("仅房主可以切换地图")));
    Btn_MapPrev->SetIsEnabled(bLocalHost);
    Btn_MapNext->SetIsEnabled(bLocalHost);

    if (CachedMapIndex == MapIndex)
    {
        return;
    }

    CachedMapIndex = MapIndex;
    MapTexture = Cast<UTexture2D>(Map->PreviewTexture.TryLoad());
    Img_MapPreview->SetBrushFromTexture(MapTexture, true);
    Txt_MapPreviewPlaceholder->SetVisibility(
        MapTexture ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
}

APSLobbyPlayerController* UPSLobbyWidget::GetLobbyController() const
{
    return Cast<APSLobbyPlayerController>(GetOwningPlayer());
}

void UPSLobbyWidget::JoinA()
{
    if (APSLobbyPlayerController* PC = GetLobbyController()) PC->ServerSetTeam(EPSLobbyTeam::TeamA);
}

void UPSLobbyWidget::JoinB()
{
    if (APSLobbyPlayerController* PC = GetLobbyController()) PC->ServerSetTeam(EPSLobbyTeam::TeamB);
}

void UPSLobbyWidget::PrevMap()
{
    if (APSLobbyPlayerController* PC = GetLobbyController()) PC->ServerChangeMap(-1);
}

void UPSLobbyWidget::NextMap()
{
    if (APSLobbyPlayerController* PC = GetLobbyController()) PC->ServerChangeMap(1);
}

void UPSLobbyWidget::StartMatch()
{
    if (APSLobbyPlayerController* PC = GetLobbyController()) PC->ServerStartMatch();
}

void UPSLobbyWidget::Leave()
{
    if (APSLobbyPlayerController* PC = GetLobbyController()) PC->ReturnToMenu();
}
