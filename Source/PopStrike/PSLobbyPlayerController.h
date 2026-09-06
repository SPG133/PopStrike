#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PSLobbyTypes.h"
#include "PSLobbyPlayerController.generated.h"

class UPSLobbyWidget;

UCLASS()
class POPSTRIKE_API APSLobbyPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    APSLobbyPlayerController();

    void ReturnToMenu();

    UFUNCTION(Server, Reliable) void ServerSubmitProfile(const FString& Name, const TArray<uint8>& Avatar);
    UFUNCTION(Server, Reliable) void ServerSetTeam(EPSLobbyTeam Team);
    UFUNCTION(Server, Reliable) void ServerChangeMap(int32 Direction);
    UFUNCTION(Server, Reliable) void ServerStartMatch();
    UFUNCTION(Client, Reliable) void ClientLobbyMessage(const FText& Message);

protected:
    virtual void BeginPlay() override;
    virtual void OnRep_PlayerState() override;

private:
    void CreateLobbyUI();
    void SubmitLocalProfile();

    UPROPERTY() TSubclassOf<UPSLobbyWidget> LobbyClass;
    UPROPERTY() TObjectPtr<UPSLobbyWidget> LobbyWidget;
    bool bProfileSubmitted = false;
    bool bServerProfileReceived = false;
    double LastActionTime = -1.0;
    bool AcceptAction();
};
