#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PSMainMenuWidget.generated.h"

class UButton;
class UEditableTextBox;
class UImage;
class UTextBlock;
class UTexture2D;

UCLASS()
class POPSTRIKE_API UPSMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    void RefreshConnection();

public:
    UFUNCTION(BlueprintCallable, Category="PopStrike|UI") void OnStartClicked();
    UFUNCTION(BlueprintCallable, Category="PopStrike|UI") void OnAvatarClicked();

    void ShowMessage(const FString& Text);
    void RestoreSavedState();

    UPROPERTY(meta=(BindWidget)) TObjectPtr<UEditableTextBox> Edt_PlayerName;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UEditableTextBox> Edt_ServerAddress;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> Txt_ErrorMessage;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> Txt_AvatarHint;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Btn_StartGame;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Btn_UploadAvatar;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UImage> Img_AvatarPreview;

    UPROPERTY(Transient) TObjectPtr<UTexture2D> AvatarTexture;
};
