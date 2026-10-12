#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PSWeaponDefinition.h"
#include "PSWeaponBase.generated.h"

class APawn;
class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class EPSWeaponSlot : uint8
{
    Primary = 0 UMETA(DisplayName="Primary"),
    Secondary = 1 UMETA(DisplayName="Secondary")
};

USTRUCT(BlueprintType)
struct POPSTRIKE_API FPSWeaponAmmo
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly, Category="Weapon") int32 Loaded = 0;
    UPROPERTY(BlueprintReadOnly, Category="Weapon") int32 Reserve = 0;
};

USTRUCT(BlueprintType)
struct POPSTRIKE_API FPSWeaponState
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly, Category="Weapon") TObjectPtr<APawn> Holder = nullptr;
    UPROPERTY(BlueprintReadOnly, Category="Weapon") bool bReloading = false;
    UPROPERTY(BlueprintReadOnly, Category="Weapon") int32 EquipSequence = 0;
    UPROPERTY(BlueprintReadOnly, Category="Weapon") int32 ActionSequence = 0;
    UPROPERTY(BlueprintReadOnly, Category="Weapon") double StartServerTime = 0.;
    UPROPERTY(BlueprintReadOnly, Category="Weapon") float Duration = 0.f;
};

// Server owns rules; Blueprint events own animation, sound, attachment and weapon-specific effects.
UCLASS(Blueprintable)
class POPSTRIKE_API APSWeaponBase : public AActor
{
    GENERATED_BODY()
public:
    APSWeaponBase();

    // Class configuration shared by server and clients, not mutable equipment state.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon") EPSWeaponSlot WeaponSlot = EPSWeaponSlot::Primary;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon") TObjectPtr<USkeletalMeshComponent> WeaponMesh;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon") TObjectPtr<UPSWeaponDefinition> Definition;
    UPROPERTY(ReplicatedUsing=OnRep_Ammo, BlueprintReadOnly, Category="Weapon") FPSWeaponAmmo Ammo;
    UPROPERTY(ReplicatedUsing=OnRep_WeaponState, BlueprintReadOnly, Category="Weapon") FPSWeaponState WeaponState;

    // Called by server-side equipment/pickup logic, never directly authorized by a client.
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Weapon") void Equip(APawn* NewHolder);
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Weapon") void Unequip();
    // One attempt per press. This base does not implement automatic trigger/hold or client prediction.
    UFUNCTION(BlueprintCallable, Category="Weapon") void Fire();
    UFUNCTION(BlueprintCallable, Category="Weapon") void Reload();
    UFUNCTION(BlueprintCallable, Category="Weapon") void CancelReload();
    UFUNCTION(BlueprintPure, Category="Weapon") bool CanFire() const;
    UFUNCTION(BlueprintPure, Category="Weapon") bool CanReload() const;
    UFUNCTION(BlueprintPure, Category="Weapon") float GetActionElapsedSeconds() const;
    // Safe to repeat when a character mesh/AnimInstance becomes ready; do not replay past sounds.
    UFUNCTION(BlueprintCallable, Category="Weapon") void RefreshPresentation();

    // Additional server-side restrictions (death, stun, active inventory slot, etc.).
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category="Weapon") bool CanUseWeapon() const;
    virtual bool CanUseWeapon_Implementation() const;
    // Authority only: attach/detach the world mesh here so attachment replication reaches clients.
    UFUNCTION(BlueprintImplementableEvent, BlueprintAuthorityOnly, Category="Weapon|Events") void OnServerHolderChanged(APawn* PreviousHolder, APawn* NewHolder);
    // Authority only: implement traces/projectiles here, using server-validated aim, never client hits.
    UFUNCTION(BlueprintImplementableEvent, BlueprintAuthorityOnly, Category="Weapon|Events") void OnServerFire(int32 ShotSequence);
    UFUNCTION(BlueprintImplementableEvent, Category="Weapon|Events") void OnAmmoChanged(FPSWeaponAmmo CurrentAmmo);
    // Called on clients and listen server, not dedicated server. Seek reload montage to ElapsedSeconds.
    UFUNCTION(BlueprintImplementableEvent, Category="Weapon|Events") void OnWeaponStateChanged(FPSWeaponState CurrentState, float ElapsedSeconds);
    UFUNCTION(BlueprintImplementableEvent, Category="Weapon|Events") void OnFireEffects(int32 ShotSequence, float ElapsedSeconds);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnRep_Owner() override;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UFUNCTION() void OnRep_Ammo();
    UFUNCTION() void OnRep_WeaponState(FPSWeaponState PreviousState);
    UFUNCTION(Server, Unreliable) void ServerFire(uint32 RequestSequence, int32 EquipSequence);
    UFUNCTION(Server, Reliable) void ServerSetReloading(bool bReloading, int32 EquipSequence);
    UFUNCTION(NetMulticast, Unreliable) void MulticastFireEffects(int32 ShotSequence, double ServerTime);
    void TryFire();
    void SetHolder(APawn* NewHolder);
    void SetReloading(bool bReloading);
    void FinishReload();
    void PublishState();
    bool HasValidHolder() const;
    bool CanSendRequest() const;
    double GetServerTime() const;
    double NextFireTime = 0.;
    uint32 LocalRequestSequence = 0;
    uint32 LastRequestSequence = 0;
    int32 LastShotSequence = 0;
    FTimerHandle ReloadTimer;
};
