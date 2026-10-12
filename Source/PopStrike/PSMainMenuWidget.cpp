#include "PSMainMenuWidget.h"

#include "Engine/Texture2D.h"
#include "PSAvatar.h"
#include "PSConnectionSubsystem.h"
#include "ImageUtils.h"

void UPSMainMenuWidget::SelectAvatar()
{
    FString Filename;

    if (!PSAvatar::PickImageFile(Filename))
    {
        return;
    }

    TArray<uint8> Bytes;
    if (!PSAvatar::EncodeFile(Filename, Bytes))
    {
        BP_OnAvatarError(FText::FromString(TEXT("请选择不超过 8MB、4096×4096 的 PNG/JPG 图片。")));
        return;
    }

    if (UGameInstance *GI = GetGameInstance())
    {
        if (UPSConnectionSubsystem *Connection =
                GI->GetSubsystem<UPSConnectionSubsystem>())
        {
            Connection->AvatarBytes = Bytes;
        }
    }

    if (UTexture2D *PreviewTexture =
            FImageUtils::ImportBufferAsTexture2D(Bytes))
    {
        BP_OnAvatarChanged(PreviewTexture);
    }
}

FString UPSMainMenuWidget::ConnectToServer(
    const FString &ServerAddress,
    const FString &PlayerName)
{
    const FString CleanName = PlayerName.TrimStartAndEnd();

    if (CleanName.IsEmpty() || CleanName.Len() > 16)
    {
        return TEXT("玩家名称必须为 1～16 个字符。");
    }

    const FString CleanAddress = ServerAddress.TrimStartAndEnd();
    if (CleanAddress.IsEmpty())
    {
        return TEXT("服务器地址不能为空。");
    }

    UGameInstance *GameInstance = GetGameInstance();
    UPSConnectionSubsystem *Connection = GameInstance
                                             ? GameInstance->GetSubsystem<UPSConnectionSubsystem>()
                                             : nullptr;
    if (!Connection || !GetOwningPlayer())
    {
        return TEXT("无法开始连接。");
    }

    Connection->SetPlayerName(CleanName);
    Connection->Connect(GetOwningPlayer(), CleanAddress);
    return {};
}
