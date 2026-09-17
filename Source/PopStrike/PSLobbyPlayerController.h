#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PSLobbyTypes.h"
#include "PSLobbyPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPSLobbyError, const FText&, Message);

UCLASS()
class POPSTRIKE_API APSLobbyPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    UFUNCTION(Server, Reliable, BlueprintCallable, Category="Lobby")
    void ServerJoinTeam(EPSLobbyTeam Team);

    UFUNCTION(Server, Reliable, BlueprintCallable, Category="Lobby")
    void ServerSelectMap(int32 MapIndex);

    UFUNCTION(Server, Reliable, BlueprintCallable, Category="Lobby")
    void ServerStartGame();

    UFUNCTION(BlueprintPure, Category="Lobby")
    bool IsRoomHost() const;

    UPROPERTY(BlueprintAssignable, Category="Lobby")
    FPSLobbyError OnLobbyError;

private:
    UFUNCTION(Client, Reliable) void ClientRequestProfile();
    UFUNCTION(Client, Reliable) void ClientLobbyError(const FText& Message);
    UFUNCTION(Server, Reliable) void ServerSubmitProfile(const FString& Name, const TArray<uint8>& Bytes);

    bool bProfileSubmitted = false;

    friend class APSLobbyGameMode;
};
