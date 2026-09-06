#include "PSMainMenuWidget.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "Misc/Paths.h"
#include "PSAvatar.h"
#include "PSConnectionSubsystem.h"
#include "PSGameConfig.h"

void UPSMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);

    Txt_ErrorMessage->SetVisibility(ESlateVisibility::Collapsed);
    RestoreSavedState();
    if (auto* Session = GetGameInstance()->GetSubsystem<UPSConnectionSubsystem>())
        Session->OnConnectionChanged.AddUObject(this, &ThisClass::RefreshConnection);
    RefreshConnection();
}

void UPSMainMenuWidget::NativeDestruct()
{
    if (GetGameInstance())
        if (auto* Session = GetGameInstance()->GetSubsystem<UPSConnectionSubsystem>()) Session->OnConnectionChanged.RemoveAll(this);
    Super::NativeDestruct();
}

void UPSMainMenuWidget::RefreshConnection()
{
    if (auto* Session = GetGameInstance()->GetSubsystem<UPSConnectionSubsystem>())
    {
        Btn_StartGame->SetIsEnabled(!Session->IsConnecting());
        Btn_UploadAvatar->SetIsEnabled(!Session->IsConnecting());
        Edt_PlayerName->SetIsEnabled(!Session->IsConnecting());
        Edt_ServerAddress->SetIsEnabled(!Session->IsConnecting());
        const FString Error = Session->ConsumeError();
        if (!Error.IsEmpty()) ShowMessage(Error);
    }
}

void UPSMainMenuWidget::RestoreSavedState()
{
    UPSConnectionSubsystem* Session = GetGameInstance()->GetSubsystem<UPSConnectionSubsystem>();
    if (!Session)
    {
        return;
    }

    if (!Session->GetPlayerName().IsEmpty())
    {
        Edt_PlayerName->SetText(FText::FromString(Session->GetPlayerName()));
    }
    Edt_ServerAddress->SetText(FText::FromString(Session->GetServerAddress()));

    AvatarTexture = PSAvatar::TextureFromBytes(Session->GetAvatarBytes());
    if (AvatarTexture)
    {
        Img_AvatarPreview->SetBrushFromTexture(AvatarTexture, true);
        Img_AvatarPreview->SetColorAndOpacity(FLinearColor::White);
    }

    const FString Error = Session->ConsumeError();
    if (!Error.IsEmpty())
    {
        ShowMessage(Error);
    }
}

void UPSMainMenuWidget::OnStartClicked()
{
    FString Name = Edt_PlayerName->GetText().ToString().TrimStartAndEnd();
    FString Address = Edt_ServerAddress->GetText().ToString().TrimStartAndEnd();

    if (Name.IsEmpty() || Name.Len() > PSConfig::MaxNameLength)
    {
        ShowMessage(TEXT("请输入 1~16 个字符的玩家名。"));
        return;
    }
    if (!UPSConnectionSubsystem::IsValidAddress(Address))
    {
        ShowMessage(TEXT("请输入服务器地址，例如 127.0.0.1:7777。"));
        return;
    }

    UPSConnectionSubsystem* Session = GetGameInstance()->GetSubsystem<UPSConnectionSubsystem>();
    APlayerController* PC = GetOwningPlayer();
    if (!Session || !PC)
    {
        ShowMessage(TEXT("无法初始化连接。"));
        return;
    }

    Session->SetPlayerName(Name);
    Session->SetServerAddress(Address);

    ShowMessage(TEXT("正在连接服务器……"));
    Session->BeginConnection(PC, Address);
}

void UPSMainMenuWidget::OnAvatarClicked()
{
    FString Filename;
    if (!PSAvatar::PickImageFile(Filename))
    {
        return;
    }

    TArray<uint8> Bytes;
    UTexture2D* Texture = nullptr;
    if (!PSAvatar::BuildNetworkAvatar(Filename, Bytes, Texture))
    {
        Txt_AvatarHint->SetText(FText::FromString(TEXT("图片无效或压缩失败。")));
        return;
    }

    AvatarTexture = Texture;
    Img_AvatarPreview->SetBrushFromTexture(AvatarTexture, true);
    Img_AvatarPreview->SetColorAndOpacity(FLinearColor::White);
    Txt_AvatarHint->SetText(FText::FromString(FPaths::GetCleanFilename(Filename)));

    if (UPSConnectionSubsystem* Session = GetGameInstance()->GetSubsystem<UPSConnectionSubsystem>())
    {
        Session->SetAvatarBytes(MoveTemp(Bytes));
    }
}

void UPSMainMenuWidget::ShowMessage(const FString& Text)
{
    Txt_ErrorMessage->SetText(FText::FromString(Text));
    Txt_ErrorMessage->SetVisibility(ESlateVisibility::Visible);
}
