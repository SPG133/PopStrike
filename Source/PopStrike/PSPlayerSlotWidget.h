#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PSLobbyTypes.h"
#include "PSPlayerSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;
class UWidget;

UCLASS()
class POPSTRIKE_API UPSPlayerSlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetEmpty();
    void SetPlayer(const FString& Name, EPSLobbyTeam Team, bool bHost, const TArray<uint8>& AvatarBytes);

private:
    void SetAvatar(const TArray<uint8>& Bytes);

    UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> Txt_PlayerName;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> Txt_PlayerState;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UWidget> Badge_Host;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UImage> Img_Avatar;

    UPROPERTY(Transient) TObjectPtr<UTexture2D> AvatarTexture;
    uint32 AvatarHash = 0;
};
