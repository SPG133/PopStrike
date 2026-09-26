#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PSLobbyTypes.h"
#include "PSMatchTypes.h"
#include "PSMatchGameMode.generated.h"

UCLASS()
class POPSTRIKE_API APSMatchGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    APSMatchGameMode();

    UPROPERTY(EditDefaultsOnly, Category="Match|Timing", meta=(ClampMin="0.01")) float FreezeDuration = 15.f;
    UPROPERTY(EditDefaultsOnly, Category="Match|Timing", meta=(ClampMin="0.01")) float DecidingRoundFreezeDuration = 20.f;
    UPROPERTY(EditDefaultsOnly, Category="Match|Timing", meta=(ClampMin="0.01")) float PlantDuration = 60.f;
    UPROPERTY(EditDefaultsOnly, Category="Match|Timing", meta=(ClampMin="0.01")) float BombDuration = 40.f;
    UPROPERTY(EditDefaultsOnly, Category="Match|Timing", meta=(ClampMin="0.01")) float DefuseDuration = 15.f;
    UPROPERTY(EditDefaultsOnly, Category="Match|Timing", meta=(ClampMin="0.01")) float RoundEndDuration = 5.f;
    UPROPERTY(EditDefaultsOnly, Category="Match", meta=(ClampMin="1")) int32 WinScore = 8;

    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
    virtual void Logout(AController* Exiting) override;
    virtual void PreLogin(const FString& Options, const FString& Address,
        const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

    UFUNCTION(BlueprintCallable, Category="Match|Objective")
    void PlantBomb(AController* Planter);

    UFUNCTION(BlueprintCallable, Category="Match|Objective")
    void StartDefuse(AController* Defuser);

    UFUNCTION(BlueprintCallable, Category="Match|Objective")
    void CancelDefuse(AController* Defuser);

private:
    void TryStartMatch();
    void EnterFreeze();
    void EnterLive();
    void RoundTimeout();
    void BombExploded();
    void FinishDefuseStage();
    void EndRound(EPSLobbyTeam Winner);
    void AdvanceRound();
    void SetPhase(EPSMatchPhase Phase, float Duration);
    EPSLobbyTeam GetTeam(const AController* Player) const;
    void ClearDefuse(bool bNotifyClients = true);

    int32 ExpectedPlayers = 1;
    TWeakObjectPtr<AController> ActiveDefuser;
    FTimerHandle PhaseTimer;
    FTimerHandle DefuseTimer;
};
