#include "PSMainMenuWidget.h"

#include "Engine/Texture2D.h"
#include "PSAvatar.h"
#include "PSConnectionSubsystem.h"

#include "IpNetDriver.h"

void UPSMainMenuWidget::SelectAvatar()
{
    FString Filename;

    if (!PSAvatar::PickImageFile(Filename))
    {
        return;
    }

    if (UGameInstance *GI = GetGameInstance())
    {
        if (UPSConnectionSubsystem *Connection =
                GI->GetSubsystem<UPSConnectionSubsystem>())
        {
            Connection->SetAvatarFilename(Filename);
        }
    }

    if (UTexture2D *PreviewTexture =
            PSAvatar::LoadPreviewTexture(Filename))
    {
        BP_OnAvatarChanged(PreviewTexture);
    }
}

FString UPSMainMenuWidget::ConnectToServer(
    const FString &ServerAddress,
    const FString &PlayerName)
{
    const UIpNetDriver* DefaultDriver = GetDefault<UIpNetDriver>();
    const FString CleanName = PlayerName.TrimStartAndEnd();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("IpNetDriver config: InitialConnectTimeout=%.2f ConnectionTimeout=%.2f"),
        DefaultDriver->InitialConnectTimeout,
        DefaultDriver->ConnectionTimeout);

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
