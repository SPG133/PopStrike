#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "PSLobbyTypes.h"
#include "PSLobbyGameState.generated.h"

class APSLobbyPlayerState;

DECLARE_MULTICAST_DELEGATE(FPSOnLobbyChanged);

UCLASS()
class POPSTRIKE_API APSLobbyGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void AddPlayerState(APlayerState* PlayerState) override;
    virtual void RemovePlayerState(APlayerState* PlayerState) override;

    APSLobbyPlayerState* GetHost() const;
    int32 CountTeam(EPSLobbyTeam Team) const;
    bool AreTeamsReady() const;
    void GetTeamPlayers(EPSLobbyTeam Team, TArray<APSLobbyPlayerState*>& OutPlayers) const;

    int32 GetSelectedMapIndex() const { return SelectedMapIndex; }
    void SetSelectedMapIndex(int32 Index);

    void NotifyLobbyChanged();
    FPSOnLobbyChanged OnLobbyChanged;

private:
    UFUNCTION() void OnRep_SelectedMapIndex();

    UPROPERTY(ReplicatedUsing=OnRep_SelectedMapIndex)
    int32 SelectedMapIndex = 0;
};
