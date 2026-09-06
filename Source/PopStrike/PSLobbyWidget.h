#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PSLobbyWidget.generated.h"

class APSLobbyGameState;
class APSLobbyPlayerController;
class UButton;
class UImage;
class UTextBlock;
class UTexture2D;
class UPSPlayerSlotWidget;

UCLASS()
class POPSTRIKE_API UPSLobbyWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void ShowServerMessage(const FText& Message);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

public:
    void BindGameState();
    void Refresh();
    void RefreshPlayers(APSLobbyGameState* GS);
    void RefreshControls(APSLobbyGameState* GS, APSLobbyPlayerController* PC);
    void RefreshMap(APSLobbyGameState* GS, bool bLocalHost);
    void ClearServerMessage();
    APSLobbyPlayerController* GetLobbyController() const;

    UFUNCTION(BlueprintCallable, Category="PopStrike|UI") void JoinA();
    UFUNCTION(BlueprintCallable, Category="PopStrike|UI") void JoinB();
    UFUNCTION(BlueprintCallable, Category="PopStrike|UI") void PrevMap();
    UFUNCTION(BlueprintCallable, Category="PopStrike|UI") void NextMap();
    UFUNCTION(BlueprintCallable, Category="PopStrike|UI") void StartMatch();
    UFUNCTION(BlueprintCallable, Category="PopStrike|UI") void Leave();

    UPROPERTY(meta=(BindWidget)) TObjectPtr<UPSPlayerSlotWidget> Slot_A1;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UPSPlayerSlotWidget> Slot_A2;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UPSPlayerSlotWidget> Slot_A3;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UPSPlayerSlotWidget> Slot_B1;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UPSPlayerSlotWidget> Slot_B2;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UPSPlayerSlotWidget> Slot_B3;

    UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Btn_JoinTeamA;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Btn_JoinTeamB;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Btn_MapPrev;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Btn_MapNext;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Btn_StartGame;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Btn_LeaveRoom;

    UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> Txt_RoomStatusHint;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> Txt_MapName;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> Txt_MapDescription;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> Txt_MapIndex;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> Txt_MapHostOnly;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> Txt_MapPreviewPlaceholder;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UImage> Img_MapPreview;

    UPROPERTY() TObjectPtr<APSLobbyGameState> BoundGameState;
    UPROPERTY(Transient) TObjectPtr<UTexture2D> MapTexture;
    FTimerHandle MessageTimer;
    FTimerHandle StateTimer;
    int32 CachedMapIndex = INDEX_NONE;
    bool bHoldingServerMessage = false;
};
