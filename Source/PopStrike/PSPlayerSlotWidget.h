#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PSLobbyTypes.h"
#include "PSPlayerSlotWidget.generated.h"

class APSLobbyPlayerState;
class UTexture2D;

UCLASS(Abstract, Blueprintable)
class POPSTRIKE_API UPSPlayerSlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="PopStrike|Lobby")
    void SetPlayer(APSLobbyPlayerState* InPlayer);

    UPROPERTY(BlueprintReadOnly, Transient, Category="PopStrike|Lobby")
    TObjectPtr<APSLobbyPlayerState> Player;

    UPROPERTY(BlueprintReadOnly, Transient, Category="PopStrike|Lobby")
    FText PlayerName;

    UPROPERTY(BlueprintReadOnly, Transient, Category="PopStrike|Lobby")
    TObjectPtr<UTexture2D> Avatar;

    UPROPERTY(BlueprintReadOnly, Transient, Category="PopStrike|Lobby")
    EPSLobbyTeam Team = EPSLobbyTeam::None;

    UPROPERTY(BlueprintReadOnly, Transient, Category="PopStrike|Lobby")
    bool bIsHost = false;

protected:
    UFUNCTION(BlueprintImplementableEvent, Category="PopStrike|Lobby", meta=(DisplayName="On Player Changed"))
    void BP_OnPlayerChanged();
};
