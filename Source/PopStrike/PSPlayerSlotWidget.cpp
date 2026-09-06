#include "PSPlayerSlotWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Misc/Crc.h"
#include "PSAvatar.h"

void UPSPlayerSlotWidget::SetEmpty()
{
    Txt_PlayerName->SetText(FText::FromString(TEXT("虚位以待")));
    Txt_PlayerState->SetText(FText::FromString(TEXT("等待玩家")));
    Badge_Host->SetVisibility(ESlateVisibility::Collapsed);
    SetAvatar({});
}

void UPSPlayerSlotWidget::SetPlayer(
    const FString& Name,
    EPSLobbyTeam Team,
    bool bHost,
    const TArray<uint8>& AvatarBytes)
{
    Txt_PlayerName->SetText(FText::FromString(Name));
    Txt_PlayerState->SetText(FText::FromString(
        Team == EPSLobbyTeam::TeamA ? TEXT("已加入烈焰突击组") : TEXT("已加入冰锋防线组")));
    Badge_Host->SetVisibility(bHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    SetAvatar(AvatarBytes);
}

void UPSPlayerSlotWidget::SetAvatar(const TArray<uint8>& Bytes)
{
    const uint32 NewHash = Bytes.IsEmpty() ? 0u : FCrc::MemCrc32(Bytes.GetData(), Bytes.Num());
    if (NewHash == AvatarHash)
    {
        return;
    }

    AvatarHash = NewHash;
    AvatarTexture = PSAvatar::TextureFromBytes(Bytes);
    Img_Avatar->SetBrushFromTexture(AvatarTexture, true);
    Img_Avatar->SetColorAndOpacity(
        AvatarTexture ? FLinearColor::White : FLinearColor(0.60f, 0.57f, 0.72f));
}
