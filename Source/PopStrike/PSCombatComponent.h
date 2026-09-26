#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/NetSerialization.h"
#include "PSCombatComponent.generated.h"

class APSWeapon;
class UPSWeaponDefinition;

UENUM(BlueprintType)
enum class EPSCombatAction : uint8 { Ready, Reloading, Dead };

USTRUCT(BlueprintType)
struct FPSCombatActionState
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) EPSCombatAction Action = EPSCombatAction::Ready;
    UPROPERTY(BlueprintReadOnly) int32 Sequence = 0;
    UPROPERTY(BlueprintReadOnly) double StartServerTime = 0;
    UPROPERTY(BlueprintReadOnly) float Duration = 0;
};

USTRUCT(BlueprintType)
struct FPSAmmoState
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) int32 Magazine = 0;
    UPROPERTY(BlueprintReadOnly) int32 Reserve = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPSCombatChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPSShotConfirmed, int32, ShotId, bool, bAccepted, bool, bDamagedTarget);

UCLASS(ClassGroup=(PopStrike), meta=(BlueprintSpawnableComponent))
class POPSTRIKE_API UPSCombatComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPSCombatComponent();
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat") TObjectPtr<UPSWeaponDefinition> StartingWeapon;
    UPROPERTY(ReplicatedUsing=OnRep_Weapon, BlueprintReadOnly, Category="Combat") TObjectPtr<APSWeapon> Weapon;
    UPROPERTY(ReplicatedUsing=OnRep_State, BlueprintReadOnly, Category="Combat") FPSCombatActionState ActionState;
    UPROPERTY(ReplicatedUsing=OnRep_Ammo, BlueprintReadOnly, Category="Combat") FPSAmmoState Ammo;
    UPROPERTY(BlueprintAssignable, Category="Combat") FPSCombatChanged OnChanged;
    UPROPERTY(BlueprintAssignable, Category="Combat") FPSShotConfirmed OnShotConfirmed;

    UFUNCTION(BlueprintCallable, Category="Combat") void StartFire();
    UFUNCTION(BlueprintCallable, Category="Combat") void StopFire();
    UFUNCTION(BlueprintCallable, Category="Combat") void Reload();
    UFUNCTION(BlueprintPure, Category="Combat") float GetActionElapsedSeconds() const;
    void CancelActions(bool bDead = false);
    void RefreshPresentation();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
    friend class FPSCombatAuthorityTest;
    void LocalFire();
    void FinishReload();
    bool CanAct() const;
    double ServerTime() const;
    int32 RoundNumber() const;
    void SetAction(EPSCombatAction Action, float Duration = 0);
    void NotifyChanged();
    UFUNCTION() void OnRep_Weapon();
    UFUNCTION() void OnRep_State();
    UFUNCTION() void OnRep_Ammo();
    UFUNCTION(Server, Unreliable) void ServerFire(int32 ShotId, FVector_NetQuantizeNormal Direction, double ShotTime, int32 Round);
    UFUNCTION(Server, Reliable) void ServerReload(int32 Round);
    UFUNCTION(Client, Unreliable) void ClientShotResult(int32 ShotId, bool bAccepted, bool bDamagedTarget);
    UFUNCTION(NetMulticast, Unreliable) void MulticastShot();
    FTimerHandle LocalFireTimer;
    FTimerHandle ReloadTimer;
    double NextLocalShotTime = 0;
    double NextServerShotTime = 0;
    int32 LocalShotId = 0;
    int32 LastServerShotId = 0;
    bool bTriggerHeld = false;
};
