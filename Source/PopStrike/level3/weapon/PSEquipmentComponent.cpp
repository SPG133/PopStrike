#include "PSEquipmentComponent.h"
#include "PSWeaponBase.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

UPSEquipmentComponent::UPSEquipmentComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

APawn* UPSEquipmentComponent::GetPawn() const
{
    return Cast<APawn>(GetOwner());
}

void UPSEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UPSEquipmentComponent, CurrentWeapon);
}

void UPSEquipmentComponent::PickupWeapon(APSWeaponBase* Weapon)
{
    const APawn* Pawn = GetPawn();
    if (!Pawn || !IsValid(Weapon)) return;
    if (Pawn->HasAuthority() || Pawn->IsLocallyControlled())
        ServerPickupWeapon(Weapon, Weapon->WeaponState.EquipSequence);
}

void UPSEquipmentComponent::ServerPickupWeapon_Implementation(APSWeaponBase* Weapon, int32 EquipSequence)
{
    const APawn* Pawn = GetPawn();
    if (!Pawn || !Pawn->HasAuthority() || !Pawn->GetController() || IsValid(CurrentWeapon)
        || !IsValid(Weapon) || IsValid(Weapon->WeaponState.Holder) || Weapon->GetOwner()
        || Weapon->WeaponState.EquipSequence != EquipSequence) return;
    // Reach, visibility, permissions and any request-rate policy are Blueprint server rules.
    OnPickupRequested.Broadcast(Weapon);
}

void UPSEquipmentComponent::DropWeapon()
{
    const APawn* Pawn = GetPawn();
    if (!Pawn || !IsValid(CurrentWeapon)) return;
    if (Pawn->HasAuthority() || Pawn->IsLocallyControlled())
        ServerDropWeapon(CurrentWeapon, CurrentWeapon->WeaponState.EquipSequence);
}

void UPSEquipmentComponent::ServerDropWeapon_Implementation(APSWeaponBase* ExpectedWeapon, int32 EquipSequence)
{
    const APawn* Pawn = GetPawn();
    // Reject requests from a previous equipment cycle, even when the same gun is picked up again.
    if (!Pawn || !Pawn->HasAuthority() || !IsValid(CurrentWeapon) || CurrentWeapon != ExpectedWeapon
        || CurrentWeapon->WeaponState.Holder != Pawn || CurrentWeapon->GetOwner() != Pawn
        || CurrentWeapon->WeaponState.EquipSequence != EquipSequence) return;
    OnDropRequested.Broadcast(CurrentWeapon);
}

bool UPSEquipmentComponent::EquipWeapon(APSWeaponBase* Weapon)
{
    APawn* Pawn = GetPawn();
    if (!Pawn || !Pawn->HasAuthority() || !Pawn->GetController() || IsValid(CurrentWeapon)
        || !IsValid(Weapon) || IsValid(Weapon->WeaponState.Holder) || Weapon->GetOwner()) return false;
    Weapon->Equip(Pawn);
    if (Weapon->WeaponState.Holder != Pawn || Weapon->GetOwner() != Pawn) return false;
    SetCurrentWeapon(Weapon);
    return true;
}

bool UPSEquipmentComponent::UnequipWeapon()
{
    const APawn* Pawn = GetPawn();
    if (!Pawn || !Pawn->HasAuthority() || !IsValid(CurrentWeapon)
        || CurrentWeapon->WeaponState.Holder != Pawn || CurrentWeapon->GetOwner() != Pawn) return false;
    CurrentWeapon->Unequip();
    SetCurrentWeapon(nullptr);
    return true;
}

void UPSEquipmentComponent::SetCurrentWeapon(APSWeaponBase* Weapon)
{
    if (CurrentWeapon == Weapon) return;
    if (IsValid(CurrentWeapon)) CurrentWeapon->OnDestroyed.RemoveDynamic(this, &ThisClass::OnWeaponDestroyed);
    CurrentWeapon = Weapon;
    if (IsValid(CurrentWeapon)) CurrentWeapon->OnDestroyed.AddDynamic(this, &ThisClass::OnWeaponDestroyed);
    OnRep_CurrentWeapon();
    GetOwner()->ForceNetUpdate();
}

void UPSEquipmentComponent::OnRep_CurrentWeapon()
{
    OnEquipmentChanged.Broadcast(CurrentWeapon);
}

void UPSEquipmentComponent::OnWeaponDestroyed(AActor* DestroyedActor)
{
    if (CurrentWeapon == DestroyedActor) SetCurrentWeapon(nullptr);
}

void UPSEquipmentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (GetOwner()->HasAuthority() && IsValid(CurrentWeapon))
    {
        if (EndPlayReason == EEndPlayReason::Destroyed) UnequipWeapon();
        if (IsValid(CurrentWeapon)) CurrentWeapon->OnDestroyed.RemoveDynamic(this, &ThisClass::OnWeaponDestroyed);
    }
    Super::EndPlay(EndPlayReason);
}
