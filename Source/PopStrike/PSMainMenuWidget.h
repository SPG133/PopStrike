#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PSMainMenuWidget.generated.h"

class UTexture2D;


UCLASS(Abstract, Blueprintable)
class POPSTRIKE_API UPSMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:

    // 选择本地头像
    UFUNCTION(BlueprintCallable, Category="PopStrike|Menu")
    void SelectAvatar();

    UFUNCTION(BlueprintCallable, Category="PopStrike|Menu", meta=(ReturnDisplayName="Error"))
    FString ConnectToServer(
        const FString& ServerAddress,
        const FString& PlayerName);


protected:


    UFUNCTION(
        BlueprintImplementableEvent,
        Category="PopStrike|Menu",
        meta=(DisplayName="On Avatar Changed"))
    void BP_OnAvatarChanged(UTexture2D* NewAvatar);

};
