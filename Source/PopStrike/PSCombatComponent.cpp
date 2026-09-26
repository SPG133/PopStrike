#include "PSCombatComponent.h"

#include "PSFPSCharacter.h"
#include "PSMatchGameState.h"
#include "PSWeapon.h"
#include "PSWeaponDefinition.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

UPSCombatComponent::UPSCombatComponent()
{
    SetIsReplicatedByDefault(true);
}

void UPSCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UPSCombatComponent, Weapon);
    DOREPLIFETIME(UPSCombatComponent, ActionState);
    DOREPLIFETIME_CONDITION(UPSCombatComponent, Ammo, COND_OwnerOnly);
}

void UPSCombatComponent::BeginPlay()
{
    Super::BeginPlay();
    if (!GetOwner()->HasAuthority() || !StartingWeapon) return;
    FActorSpawnParameters Parameters;
    Parameters.Owner = GetOwner();
    Parameters.Instigator = Cast<APawn>(GetOwner());
    Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    const TSubclassOf<APSWeapon> Class = StartingWeapon->WeaponClass
        ? StartingWeapon->WeaponClass.Get() : APSWeapon::StaticClass();
    Weapon = GetWorld()->SpawnActor<APSWeapon>(Class, Parameters);
    if (!Weapon) return;
    Weapon->Initialize(StartingWeapon);
    Ammo.Magazine = FMath::Max(1, StartingWeapon->MagazineSize);
    Ammo.Reserve = FMath::Max(0, StartingWeapon->StartingReserve);
    NotifyChanged();
}

void UPSCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
    if (GetOwner()->HasAuthority() && IsValid(Weapon)) Weapon->Destroy();
    Super::EndPlay(EndPlayReason);
}

double UPSCombatComponent::ServerTime() const
{
    const auto* State = GetWorld()->GetGameState<AGameStateBase>();
    return State ? State->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
}

int32 UPSCombatComponent::RoundNumber() const
{
    const auto* State = GetWorld()->GetGameState<APSMatchGameState>();
    return State ? State->MatchState.RoundNumber : 0;
}

bool UPSCombatComponent::CanAct() const
{
    const auto* Character = Cast<APSFPSCharacter>(GetOwner());
    const auto* State = GetWorld()->GetGameState<APSMatchGameState>();
    return Character && Character->IsAlive() && Character->GetController() && Weapon && Weapon->Definition && State &&
        (State->MatchState.Phase == EPSMatchPhase::Live || State->MatchState.Phase == EPSMatchPhase::BombPlanted);
}

void UPSCombatComponent::StartFire()
{
    const auto* Character = Cast<APawn>(GetOwner());
    if (!Character || !Character->IsLocallyControlled() || bTriggerHeld || !CanAct() ||
        ActionState.Action != EPSCombatAction::Ready) return;
    bTriggerHeld = true;
    LocalFire();
    if (bTriggerHeld)
    {
        GetWorld()->GetTimerManager().SetTimer(LocalFireTimer, this, &ThisClass::LocalFire,
            FMath::Max(0.05f, Weapon->Definition->FireInterval), true);
    }
}

void UPSCombatComponent::StopFire()
{
    bTriggerHeld = false;
    GetWorld()->GetTimerManager().ClearTimer(LocalFireTimer);
}

void UPSCombatComponent::LocalFire()
{
    if (!bTriggerHeld || !CanAct() || ActionState.Action != EPSCombatAction::Ready || Ammo.Magazine <= 0)
    {
        StopFire();
        return;
    }
    const double Now = GetWorld()->GetTimeSeconds();
    if (Now + 0.001 < NextLocalShotTime) return;
    NextLocalShotTime = Now + FMath::Max(0.05f, Weapon->Definition->FireInterval);
    const auto* Pawn = CastChecked<APawn>(GetOwner());
    // Only presentation is predicted. The ammo UI always consumes the authoritative snapshot.
    if (GetNetMode() != NM_DedicatedServer) Weapon->PresentShot(true);
    ServerFire(++LocalShotId, Pawn->GetBaseAimRotation().Vector(), ServerTime(), RoundNumber());
}

void UPSCombatComponent::ServerFire_Implementation(int32 ShotId, FVector_NetQuantizeNormal Direction,
    double ShotTime, int32 Round)
{
    if (ShotId <= LastServerShotId) return;
    LastServerShotId = ShotId;
    const double Now = ServerTime();
    const auto* Pawn = Cast<APawn>(GetOwner());
    const FVector Aim = FVector(Direction);
    const bool bValidAim = !Aim.ContainsNaN() && FMath::IsNearlyEqual(Aim.SizeSquared(), 1.0, 0.02) &&
        Pawn && FVector::DotProduct(Aim, Pawn->GetBaseAimRotation().Vector()) >= 0.8;
    if (!CanAct() || ActionState.Action != EPSCombatAction::Ready || Ammo.Magazine <= 0 ||
        Round != RoundNumber() || !FMath::IsFinite(ShotTime) || ShotTime < Now - 1.0 || ShotTime > Now + 0.25 ||
        !bValidAim || Now + 0.005 < NextServerShotTime)
    {
        ClientShotResult(ShotId, false, false);
        return;
    }
    NextServerShotTime = FMath::Max(Now, NextServerShotTime) +
        FMath::Max(0.05f, Weapon->Definition->FireInterval);
    --Ammo.Magazine;

    FCollisionQueryParams Query(SCENE_QUERY_STAT(PSRifleShot), true, GetOwner());
    Query.AddIgnoredActor(Weapon);
    const FVector Eye = Pawn->GetPawnViewLocation();
    const float Range = FMath::Clamp(Weapon->Definition->Range, 100.f, 100000.f);
    FHitResult SightHit;
    const FVector SightEnd = Eye + Aim * Range;
    GetWorld()->LineTraceSingleByChannel(SightHit, Eye, SightEnd, ECC_Visibility, Query);
    const FVector Target = SightHit.bBlockingHit ? SightHit.ImpactPoint : SightEnd;
    const FVector Muzzle = Weapon->GetMuzzleLocation();
    FHitResult Hit;
    // A camera outside cover must never allow the barrel to shoot through that cover.
    if (!GetWorld()->LineTraceSingleByChannel(Hit, Eye, Muzzle, ECC_Visibility, Query))
    {
        GetWorld()->LineTraceSingleByChannel(Hit, Muzzle, Target + Aim * 2.f, ECC_Visibility, Query);
    }
    bool bDamaged = false;
    if (Hit.bBlockingHit && Hit.GetActor())
    {
        bDamaged = UGameplayStatics::ApplyPointDamage(Hit.GetActor(),
            FMath::Max(0.f, Weapon->Definition->Damage), Aim, Hit,
            Pawn->GetController(), Weapon, nullptr) > 0.f;
    }
    NotifyChanged();
    MulticastShot();
    ClientShotResult(ShotId, true, bDamaged);
}

void UPSCombatComponent::MulticastShot_Implementation()
{
    const auto* Pawn = Cast<APawn>(GetOwner());
    if (GetNetMode() != NM_DedicatedServer && Pawn && !Pawn->IsLocallyControlled() && Weapon)
        Weapon->PresentShot(false);
}

void UPSCombatComponent::ClientShotResult_Implementation(int32 ShotId, bool bAccepted, bool bDamagedTarget)
{
    OnShotConfirmed.Broadcast(ShotId, bAccepted, bDamagedTarget);
}

void UPSCombatComponent::Reload()
{
    const auto* Pawn = Cast<APawn>(GetOwner());
    if (!Pawn || !Pawn->IsLocallyControlled()) return;
    StopFire();
    ServerReload(RoundNumber());
}

void UPSCombatComponent::ServerReload_Implementation(int32 Round)
{
    if (!CanAct() || Round != RoundNumber() || ActionState.Action != EPSCombatAction::Ready ||
        Ammo.Reserve <= 0 || Ammo.Magazine >= Weapon->Definition->MagazineSize) return;
    const float Duration = FMath::Max(0.1f, Weapon->Definition->ReloadDuration);
    SetAction(EPSCombatAction::Reloading, Duration);
    GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &ThisClass::FinishReload, Duration, false);
}

void UPSCombatComponent::FinishReload()
{
    if (ActionState.Action != EPSCombatAction::Reloading) return;
    if (CanAct())
    {
        const int32 Transfer = FMath::Clamp(Weapon->Definition->MagazineSize - Ammo.Magazine, 0, Ammo.Reserve);
        Ammo.Magazine += Transfer;
        Ammo.Reserve -= Transfer;
    }
    SetAction(EPSCombatAction::Ready);
}

void UPSCombatComponent::CancelActions(bool bDead)
{
    StopFire();
    if (!GetOwner()->HasAuthority()) return;
    GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
    SetAction(bDead ? EPSCombatAction::Dead : EPSCombatAction::Ready);
}

void UPSCombatComponent::SetAction(EPSCombatAction Action, float Duration)
{
    ActionState.Action = Action;
    ++ActionState.Sequence;
    ActionState.StartServerTime = ServerTime();
    ActionState.Duration = Duration;
    NotifyChanged();
    RefreshPresentation();
}

float UPSCombatComponent::GetActionElapsedSeconds() const
{
    return FMath::Clamp(ServerTime() - ActionState.StartServerTime, 0.0, double(ActionState.Duration));
}

void UPSCombatComponent::RefreshPresentation()
{
    if (GetNetMode() == NM_DedicatedServer || !Weapon || !Weapon->Definition) return;
    Weapon->PresentReload(ActionState.Action == EPSCombatAction::Reloading,
        GetActionElapsedSeconds(), ActionState.Duration);
}

void UPSCombatComponent::NotifyChanged()
{
    GetOwner()->ForceNetUpdate();
    OnChanged.Broadcast();
}

void UPSCombatComponent::OnRep_State()
{
    if (ActionState.Action != EPSCombatAction::Ready) StopFire();
    OnChanged.Broadcast();
    RefreshPresentation();
}

void UPSCombatComponent::OnRep_Ammo()
{
    OnChanged.Broadcast();
}

void UPSCombatComponent::OnRep_Weapon()
{
    if (Weapon) Weapon->RefreshPresentation();
    OnChanged.Broadcast();
}
