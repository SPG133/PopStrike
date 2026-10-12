#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PSEquipmentComponent.generated.h"

class APawn;
class APSWeaponBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPSEquipmentWeaponEvent, APSWeaponBase*, Weapon);

// Network boundary only. Blueprint owns interaction rules, placement and presentation.
UCLASS(Blueprintable, ClassGroup=(Equipment), meta=(BlueprintSpawnableComponent))
class POPSTRIKE_API UPSEquipmentComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPSEquipmentComponent();

    // Server-only requests. Blueprint validates gameplay rules before calling Equip/UnequipWeapon.
    UPROPERTY(BlueprintAssignable, Category="Equipment|Server") FPSEquipmentWeaponEvent OnPickupRequested;
    UPROPERTY(BlueprintAssignable, Category="Equipment|Server") FPSEquipmentWeaponEvent OnDropRequested;
    // Current snapshot notification on server and relevant clients; safe to query again after BeginPlay.
    UPROPERTY(BlueprintAssignable, Category="Equipment") FPSEquipmentWeaponEvent OnEquipmentChanged;
    UPROPERTY(ReplicatedUsing=OnRep_CurrentWeapon, BlueprintReadWrite, Category="Weapon")
    TObjectPtr<APSWeaponBase> CurrentWeapon = nullptr;
    UFUNCTION(BlueprintPure, Category="Equipment") APSWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }
    UFUNCTION(BlueprintCallable, Category="Equipment") void PickupWeapon(APSWeaponBase* Weapon);
    UFUNCTION(BlueprintCallable, Category="Equipment") void DropWeapon();
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Equipment|Server") bool EquipWeapon(APSWeaponBase* Weapon);
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Equipment|Server") bool UnequipWeapon();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UFUNCTION() void OnRep_CurrentWeapon();
    UFUNCTION() void OnWeaponDestroyed(AActor* DestroyedActor);
    UFUNCTION(Server, Reliable) void ServerPickupWeapon(APSWeaponBase* Weapon, int32 EquipSequence);
    UFUNCTION(Server, Reliable) void ServerDropWeapon(APSWeaponBase* ExpectedWeapon, int32 EquipSequence);
    APawn* GetPawn() const;
    void SetCurrentWeapon(APSWeaponBase* Weapon);
};
