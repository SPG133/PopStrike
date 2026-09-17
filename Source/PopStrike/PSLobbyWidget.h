#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PSLobbyTypes.h"
#include "PSLobbyWidget.generated.h"

class APSLobbyGameState;
class APSLobbyPlayerController;
class APSLobbyPlayerState;

UCLASS(Abstract, Blueprintable)
class POPSTRIKE_API UPSLobbyWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, Transient, Category="PopStrike|Lobby")
    TObjectPtr<APSLobbyGameState> LobbyState;

    UPROPERTY(BlueprintReadOnly, Transient, Category="PopStrike|Lobby")
    TObjectPtr<APSLobbyPlayerController> LobbyController;

    UPROPERTY(BlueprintReadOnly, Transient, Category="PopStrike|Lobby")
    TArray<APSLobbyPlayerState*> TeamA;

    UPROPERTY(BlueprintReadOnly, Transient, Category="PopStrike|Lobby")
    TArray<APSLobbyPlayerState*> TeamB;

    UPROPERTY(BlueprintReadOnly, Transient, Category="PopStrike|Lobby")
    FPSLobbyMap CurrentMap;

    UPROPERTY(BlueprintReadOnly, Transient, Category="PopStrike|Lobby")
    bool bHasMap = false;

    UPROPERTY(BlueprintReadOnly, Transient, Category="PopStrike|Lobby")
    bool bIsHost = false;

    UPROPERTY(BlueprintReadOnly, Transient, Category="PopStrike|Lobby")
    bool bCanInteract = false;

    UFUNCTION(BlueprintCallable, Category="PopStrike|Lobby")
    void JoinTeam(EPSLobbyTeam Team);

    UFUNCTION(BlueprintCallable, Category="PopStrike|Lobby")
    void ChangeMap(int32 Offset);

    UFUNCTION(BlueprintCallable, Category="PopStrike|Lobby")
    void StartGame();

    UFUNCTION(BlueprintCallable, Category="PopStrike|Lobby")
    void RefreshLobby();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UFUNCTION(BlueprintImplementableEvent, Category="PopStrike|Lobby", meta=(DisplayName="On Lobby Refreshed"))
    void BP_OnLobbyRefreshed();

    UFUNCTION(BlueprintImplementableEvent, Category="PopStrike|Lobby", meta=(DisplayName="On Lobby Error"))
    void BP_OnLobbyError(const FText& Message);

private:
    UFUNCTION() void HandleLobbyError(const FText& Message);
};
