#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PSLobbyTypes.h"
#include "PSLobbyPlayerState.generated.h"

UCLASS()
class POPSTRIKE_API APSLobbyPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnRep_PlayerName() override;

    EPSLobbyTeam GetTeam() const { return Team; }
    int32 GetJoinOrder() const { return JoinOrder; }
    const TArray<uint8>& GetAvatarBytes() const { return AvatarBytes; }

    void SetTeam(EPSLobbyTeam Value);
    void SetJoinOrder(int32 Value);
    void SetProfile(const FString& Name, const TArray<uint8>& Bytes);

private:
    UFUNCTION() void OnRep_LobbyState();
    UFUNCTION() void OnRep_Avatar();
    void NotifyLobbyChanged() const;

    UPROPERTY(ReplicatedUsing=OnRep_LobbyState)
    EPSLobbyTeam Team = EPSLobbyTeam::None;

    UPROPERTY(ReplicatedUsing=OnRep_LobbyState)
    int32 JoinOrder = INDEX_NONE;

    UPROPERTY(ReplicatedUsing=OnRep_Avatar)
    TArray<uint8> AvatarBytes;
};
