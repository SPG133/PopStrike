#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "PSLobbyTypes.h"
#include "PSLobbyGameState.generated.h"

class APSLobbyPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPSLobbyChanged);

UCLASS()
class POPSTRIKE_API APSLobbyGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    UPROPERTY(ReplicatedUsing=OnRep_Data, BlueprintReadOnly, Category="Lobby")
    TArray<FPSLobbyMap> Maps;

    UPROPERTY(ReplicatedUsing=OnRep_Data, BlueprintReadOnly, Category="Lobby")
    int32 SelectedMap = 0;

    UPROPERTY(ReplicatedUsing=OnRep_Data, BlueprintReadOnly, Category="Lobby")
    TObjectPtr<APSLobbyPlayerState> Host;

    UPROPERTY(ReplicatedUsing=OnRep_Data, BlueprintReadOnly, Category="Lobby")
    bool bStarting = false;

    UPROPERTY(BlueprintAssignable, Category="Lobby")
    FPSLobbyChanged OnChanged;

    UFUNCTION(BlueprintPure, Category="Lobby")
    TArray<APSLobbyPlayerState*> GetTeamPlayers(EPSLobbyTeam Team) const;

    void BroadcastChanged();
    virtual void AddPlayerState(APlayerState* PlayerState) override;
    virtual void RemovePlayerState(APlayerState* PlayerState) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UFUNCTION() void OnRep_Data();
};
