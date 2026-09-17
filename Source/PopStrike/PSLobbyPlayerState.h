#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PSLobbyTypes.h"
#include "PSLobbyPlayerState.generated.h"

class UTexture2D;

UCLASS()
class POPSTRIKE_API APSLobbyPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    UPROPERTY(ReplicatedUsing = OnRep_Data, BlueprintReadOnly, Category = "Lobby")
    EPSLobbyTeam Team = EPSLobbyTeam::None;

    UPROPERTY(ReplicatedUsing = OnRep_Data, BlueprintReadOnly, Category = "Lobby")
    bool bIsHost = false;

    UFUNCTION(BlueprintPure, Category = "Lobby")
    UTexture2D *GetAvatar();

    virtual void OnRep_PlayerName() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

private:
    UPROPERTY(ReplicatedUsing = OnRep_Avatar)
    TArray<uint8> AvatarBytes;

    UPROPERTY(Transient)
    TObjectPtr<UTexture2D> AvatarTexture;

    UFUNCTION()
    void OnRep_Data();
    UFUNCTION()
    void OnRep_Avatar();

    friend class APSLobbyPlayerController;
};
