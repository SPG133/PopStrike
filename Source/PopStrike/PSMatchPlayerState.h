#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PSLobbyTypes.h"
#include "PSMatchPlayerState.generated.h"

class UTexture2D;

UCLASS()
class POPSTRIKE_API APSMatchPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    UPROPERTY(Replicated, BlueprintReadOnly, Category="Match")
    EPSLobbyTeam Team = EPSLobbyTeam::None;

    UFUNCTION(BlueprintPure, Category="Match")
    UTexture2D* GetAvatar();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(ReplicatedUsing=OnRep_Avatar)
    TArray<uint8> AvatarBytes;

    UPROPERTY(Transient)
    TObjectPtr<UTexture2D> AvatarTexture;

    UFUNCTION() void OnRep_Avatar();

    friend class APSLobbyPlayerState;
};
