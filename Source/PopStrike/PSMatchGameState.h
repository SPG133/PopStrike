#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "PSMatchTypes.h"
#include "PSMatchGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPSMatchChanged);

UCLASS()
class POPSTRIKE_API APSMatchGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    UPROPERTY(ReplicatedUsing=OnRep_MatchState, BlueprintReadOnly, Category="Match")
    FPSMatchState MatchState;

    UPROPERTY(BlueprintAssignable, Category="Match")
    FPSMatchChanged OnChanged;

    UFUNCTION(BlueprintPure, Category="Match")
    float GetPhaseSecondsRemaining() const;

    UFUNCTION(BlueprintPure, Category="Match")
    float GetDefuseStageSecondsRemaining() const;

    void NotifyChanged();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UFUNCTION() void OnRep_MatchState();
};
