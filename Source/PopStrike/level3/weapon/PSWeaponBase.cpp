#include "PSWeaponBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

APSWeaponBase::APSWeaponBase()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(true);
    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    SetRootComponent(WeaponMesh);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APSWeaponBase::BeginPlay()
{
    Super::BeginPlay();
    if (Definition)
    {
        WeaponMesh->SetSkeletalMesh(Definition->Data.WeaponMesh);
        if (Definition->Data.WeaponAnimClass) WeaponMesh->SetAnimInstanceClass(Definition->Data.WeaponAnimClass);
        if (HasAuthority())
        {
            const int32 Capacity = FMath::Clamp(Definition->Data.MagazineCapacity, 1, 10000);
            Ammo.Loaded = Capacity;
            Ammo.Reserve = Capacity * FMath::Clamp(Definition->Data.SpareMagazineCount, 0, 10000);
            OnRep_Ammo();
        }
    }
    RefreshPresentation();
}

void APSWeaponBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(ReloadTimer);
    Super::EndPlay(EndPlayReason);
}

void APSWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(APSWeaponBase, Ammo, COND_OwnerOnly);
    DOREPLIFETIME(APSWeaponBase, WeaponState);
}

bool APSWeaponBase::HasValidHolder() const
{
    return IsValid(WeaponState.Holder) && GetOwner() == WeaponState.Holder
        && WeaponState.Holder->GetController() != nullptr;
}

bool APSWeaponBase::CanSendRequest() const
{
    return HasValidHolder() && WeaponState.Holder->IsLocallyControlled();
}

bool APSWeaponBase::CanUseWeapon_Implementation() const { return true; }

bool APSWeaponBase::CanFire() const
{
    return Definition && HasValidHolder() && !WeaponState.bReloading && Ammo.Loaded > 0
        && FMath::IsFinite(Definition->Data.FireRateRPM) && Definition->Data.FireRateRPM > 0.f
        && GetServerTime() >= NextFireTime && CanUseWeapon();
}

bool APSWeaponBase::CanReload() const
{
    return Definition && HasValidHolder() && !WeaponState.bReloading && Ammo.Reserve > 0
        && Ammo.Loaded < FMath::Clamp(Definition->Data.MagazineCapacity, 1, 10000)
        && FMath::IsFinite(Definition->Data.ReloadDuration) && Definition->Data.ReloadDuration > 0.f
        && CanUseWeapon();
}

void APSWeaponBase::Equip(APawn* NewHolder)
{
    if (HasAuthority() && IsValid(NewHolder) && NewHolder->GetController()) SetHolder(NewHolder);
}

void APSWeaponBase::Unequip()
{
    if (HasAuthority()) SetHolder(nullptr);
}

void APSWeaponBase::SetHolder(APawn* NewHolder)
{
    if (WeaponState.Holder == NewHolder) return;
    GetWorldTimerManager().ClearTimer(ReloadTimer);
    APawn* PreviousHolder = WeaponState.Holder;
    WeaponState.Holder = NewHolder;
    WeaponState.EquipSequence = WeaponState.EquipSequence == MAX_int32 ? 1 : WeaponState.EquipSequence + 1;
    LastRequestSequence = 0;
    WeaponState.bReloading = false;
    WeaponState.Duration = 0.f;
    SetOwner(NewHolder);
    bNetUseOwnerRelevancy = NewHolder != nullptr;
    bOnlyRelevantToOwner = false;
    // Blueprint may opt into AlwaysRelevant for ground weapons; normal distance relevancy is the default.
    FlushNetDormancy();
    OnServerHolderChanged(PreviousHolder, NewHolder);
    PublishState();
}

void APSWeaponBase::Fire()
{
    if (HasAuthority()) TryFire();
    else if (CanSendRequest()) ServerFire(++LocalRequestSequence, WeaponState.EquipSequence);
}

void APSWeaponBase::ServerFire_Implementation(uint32 RequestSequence, int32 EquipSequence)
{
    if (!HasValidHolder() || EquipSequence != WeaponState.EquipSequence) return;
    // Reject reordered/duplicate requests, allowing uint32 wrap within an equipment session.
    const uint32 Delta = RequestSequence - LastRequestSequence;
    if (Delta == 0 || Delta >= 0x80000000u) return;
    LastRequestSequence = RequestSequence;
    TryFire();
}

void APSWeaponBase::TryFire()
{
    if (!HasAuthority() || !CanFire()) return;
    NextFireTime = GetServerTime() + 60. / FMath::Clamp(Definition->Data.FireRateRPM, 1.f, 6000.f);
    --Ammo.Loaded;
    LastShotSequence = LastShotSequence == MAX_int32 ? 1 : LastShotSequence + 1;
    OnRep_Ammo();
    ForceNetUpdate();
    MulticastFireEffects(LastShotSequence, GetServerTime());
    OnServerFire(LastShotSequence);
}

void APSWeaponBase::MulticastFireEffects_Implementation(int32 Sequence, double ServerTime)
{
    const float Elapsed = static_cast<float>(FMath::Max(0., GetServerTime() - ServerTime));
    // Do not replay stale shots after a network stall. Effects are not gameplay state.
    if (GetNetMode() != NM_DedicatedServer && Elapsed <= 0.5f) OnFireEffects(Sequence, Elapsed);
}

void APSWeaponBase::Reload() { SetReloading(true); }
void APSWeaponBase::CancelReload() { SetReloading(false); }

void APSWeaponBase::SetReloading(bool bReloading)
{
    if (!HasAuthority())
    {
        if (CanSendRequest()) ServerSetReloading(bReloading, WeaponState.EquipSequence);
        return;
    }
    if (WeaponState.bReloading == bReloading || (bReloading && !CanReload())) return;
    GetWorldTimerManager().ClearTimer(ReloadTimer);
    WeaponState.bReloading = bReloading;
    WeaponState.Duration = bReloading ? FMath::Clamp(Definition->Data.ReloadDuration, 0.01f, 300.f) : 0.f;
    if (bReloading)
        GetWorldTimerManager().SetTimer(ReloadTimer, this, &APSWeaponBase::FinishReload, WeaponState.Duration, false);
    PublishState();
}

void APSWeaponBase::ServerSetReloading_Implementation(bool bReloading, int32 EquipSequence)
{
    if (HasValidHolder() && EquipSequence == WeaponState.EquipSequence) SetReloading(bReloading);
}

void APSWeaponBase::FinishReload()
{
    if (!HasAuthority() || !WeaponState.bReloading) return;
    if (Definition && HasValidHolder() && CanUseWeapon())
    {
        const int32 Needed = FMath::Max(0, FMath::Clamp(Definition->Data.MagazineCapacity, 1, 10000) - Ammo.Loaded);
        const int32 Added = FMath::Min(Needed, Ammo.Reserve);
        Ammo.Loaded += Added;
        Ammo.Reserve -= Added;
        OnRep_Ammo();
    }
    SetReloading(false);
}

double APSWeaponBase::GetServerTime() const
{
    if (const AGameStateBase* State = GetWorld()->GetGameState()) return State->GetServerWorldTimeSeconds();
    return GetWorld()->GetTimeSeconds();
}

float APSWeaponBase::GetActionElapsedSeconds() const
{
    return static_cast<float>(FMath::Clamp(GetServerTime() - WeaponState.StartServerTime, 0., static_cast<double>(WeaponState.Duration)));
}

void APSWeaponBase::PublishState()
{
    WeaponState.ActionSequence = WeaponState.ActionSequence == MAX_int32 ? 1 : WeaponState.ActionSequence + 1;
    WeaponState.StartServerTime = GetServerTime();
    RefreshPresentation();
    ForceNetUpdate();
}

void APSWeaponBase::OnRep_Ammo() { OnAmmoChanged(Ammo); }
void APSWeaponBase::OnRep_WeaponState(FPSWeaponState PreviousState)
{
    if (PreviousState.EquipSequence != WeaponState.EquipSequence) LocalRequestSequence = 0;
    RefreshPresentation();
}
void APSWeaponBase::OnRep_Owner()
{
    Super::OnRep_Owner();
    RefreshPresentation();
}

void APSWeaponBase::RefreshPresentation()
{
    if (HasActorBegunPlay() && GetNetMode() != NM_DedicatedServer)
        OnWeaponStateChanged(WeaponState, GetActionElapsedSeconds());
}
