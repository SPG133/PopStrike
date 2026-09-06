#include "PSLobbyPlayerController.h"

#include "PSConnectionSubsystem.h"
#include "PSAvatar.h"
#include "PSGameConfig.h"
#include "PSLobbyGameMode.h"
#include "PSLobbyPlayerState.h"
#include "PSLobbyWidget.h"
#include "UObject/ConstructorHelpers.h"

APSLobbyPlayerController::APSLobbyPlayerController()
{
    static ConstructorHelpers::FClassFinder<UUserWidget> LobbyBP(PSConfig::LobbyWidget);
    if (LobbyBP.Class && LobbyBP.Class->IsChildOf(UPSLobbyWidget::StaticClass())) LobbyClass = LobbyBP.Class.Get();
}

void APSLobbyPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (!IsLocalController() || GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    CreateLobbyUI();
    if (auto* Session = GetGameInstance()->GetSubsystem<UPSConnectionSubsystem>()) Session->MarkConnected();
    SubmitLocalProfile();
}

void APSLobbyPlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    SubmitLocalProfile();
}

void APSLobbyPlayerController::CreateLobbyUI()
{
    if (LobbyWidget || !LobbyClass)
    {
        return;
    }

    LobbyWidget = CreateWidget<UPSLobbyWidget>(this, LobbyClass);
    if (!LobbyWidget)
    {
        return;
    }

    LobbyWidget->AddToViewport(100);
    bShowMouseCursor = true;

    FInputModeUIOnly Mode;
    Mode.SetWidgetToFocus(LobbyWidget->TakeWidget());
    SetInputMode(Mode);
}

void APSLobbyPlayerController::SubmitLocalProfile()
{
    if (bProfileSubmitted || !IsLocalController() || !GetPlayerState<APSLobbyPlayerState>())
    {
        return;
    }

    UPSConnectionSubsystem* Session = GetGameInstance()->GetSubsystem<UPSConnectionSubsystem>();
    if (!Session)
    {
        return;
    }

    const FString Name = Session->GetPlayerName().IsEmpty() ? TEXT("Player") : Session->GetPlayerName();
    ServerSubmitProfile(Name, Session->GetAvatarBytes());
    bProfileSubmitted = true;
}

void APSLobbyPlayerController::ServerSubmitProfile_Implementation(const FString& InName, const TArray<uint8>& InAvatar)
{
    if (bServerProfileReceived) return;
    bServerProfileReceived = true;
    APSLobbyPlayerState* PS = GetPlayerState<APSLobbyPlayerState>();
    if (!PS)
    {
        return;
    }

    FString Name = InName.TrimStartAndEnd().Left(PSConfig::MaxNameLength);
    if (Name.IsEmpty())
    {
        Name = TEXT("Player");
    }

    const TArray<uint8> Empty;
    PS->SetProfile(Name, PSAvatar::ValidateNetworkAvatar(InAvatar) ? InAvatar : Empty);
}

void APSLobbyPlayerController::ServerSetTeam_Implementation(EPSLobbyTeam Team)
{
    if (!AcceptAction()) return;
    if (APSLobbyGameMode* GM = GetWorld()->GetAuthGameMode<APSLobbyGameMode>())
    {
        GM->TrySetTeam(this, Team);
    }
}

void APSLobbyPlayerController::ServerChangeMap_Implementation(int32 Direction)
{
    if (!AcceptAction()) return;
    if (APSLobbyGameMode* GM = GetWorld()->GetAuthGameMode<APSLobbyGameMode>())
    {
        GM->TryChangeMap(this, Direction);
    }
}

void APSLobbyPlayerController::ServerStartMatch_Implementation()
{
    if (!AcceptAction()) return;
    if (APSLobbyGameMode* GM = GetWorld()->GetAuthGameMode<APSLobbyGameMode>())
    {
        GM->TryStartMatch(this);
    }
}

void APSLobbyPlayerController::ClientLobbyMessage_Implementation(const FText& Message)
{
    if (LobbyWidget)
    {
        LobbyWidget->ShowServerMessage(Message);
    }
}

void APSLobbyPlayerController::ReturnToMenu()
{
    if (IsLocalController())
    {
        ClientTravel(PSConfig::MainMenuMap, TRAVEL_Absolute);
    }
}


bool APSLobbyPlayerController::AcceptAction()
{
    const double Now = FPlatformTime::Seconds();
    if (Now - LastActionTime < 0.2) return false;
    LastActionTime = Now;
    return true;
}
