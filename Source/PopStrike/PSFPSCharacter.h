#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PSFPSCharacter.generated.h"

class UCameraComponent;
class UPSCombatComponent;

UCLASS()
class POPSTRIKE_API APSFPSCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    APSFPSCharacter();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FPS") TObjectPtr<UCameraComponent> Camera;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FPS") TObjectPtr<UPSCombatComponent> Combat;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS", meta=(ClampMin="1")) float MaxHealth = 100.f;
    UPROPERTY(ReplicatedUsing=OnRep_Health, BlueprintReadOnly, Category="FPS") float Health = 100.f;
    UFUNCTION(BlueprintPure, Category="FPS") bool IsAlive() const { return Health > 0.f; }
    UFUNCTION(BlueprintPure, Category="FPS") bool CanMove() const;
    UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category="FPS|Presentation") void PresentHealth(float PreviousHealth);
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void PawnClientRestart() override;
    virtual void UnPossessed() override;
    virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
    virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
    UFUNCTION() void UpdateMatchPhase();
    UFUNCTION() void OnRep_Health(float PreviousHealth);
};
