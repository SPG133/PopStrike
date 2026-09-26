#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "PSCombatComponent.h"
#include "PSFPSCharacter.h"
#include "PSMatchGameState.h"
#include "PSWeaponDefinition.h"
#include "GameFramework/PlayerController.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPSCombatAuthorityTest, "PopStrike.Combat.AuthorityAndRecovery",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPSCombatAuthorityTest::RunTest(const FString& Parameters)
{
    auto* Definition = LoadObject<UPSWeaponDefinition>(nullptr, TEXT("/Game/PopStrike/Data/DA_Rifle.DA_Rifle"));
    if (!TestNotNull(TEXT("Authored rifle definition loads"), Definition)) return false;
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    auto* State = World->SpawnActor<APSMatchGameState>();
    World->SetGameState(State);
    State->MatchState.Phase = EPSMatchPhase::Live;
    State->MatchState.RoundNumber = 1;
    auto* Controller = World->SpawnActor<APlayerController>();
    auto* Character = World->SpawnActorDeferred<APSFPSCharacter>(APSFPSCharacter::StaticClass(), FTransform::Identity);
    Character->Combat->StartingWeapon = Definition;
    Character->FinishSpawning(FTransform::Identity);
    Controller->Possess(Character);
    Character->DispatchBeginPlay();
    UPSCombatComponent* Combat = Character->Combat.Get();
    if (!TestNotNull(TEXT("Authority equips a weapon"), Combat->Weapon.Get()))
    {
        World->DestroyWorld(false);
        return false;
    }
    const int32 FullMagazine = Combat->Ammo.Magazine;
    Combat->ServerFire_Implementation(1, FVector::ForwardVector, Combat->ServerTime(), 1);
    TestEqual(TEXT("Valid shot consumes exactly one round"), Combat->Ammo.Magazine, FullMagazine - 1);
    Combat->ServerFire_Implementation(1, FVector::ForwardVector, Combat->ServerTime(), 1);
    TestEqual(TEXT("Duplicate shot cannot consume ammunition twice"), Combat->Ammo.Magazine, FullMagazine - 1);
    Combat->ServerFire_Implementation(2, FVector::ForwardVector, Combat->ServerTime(), 1);
    TestEqual(TEXT("Excessive fire rate rejected"), Combat->Ammo.Magazine, FullMagazine - 1);

    Combat->NextServerShotTime = 0;
    Combat->ServerFire_Implementation(3, FVector::ForwardVector, Combat->ServerTime() - 2, 1);
    Combat->ServerFire_Implementation(4, -FVector::ForwardVector, Combat->ServerTime(), 1);
    Combat->ServerFire_Implementation(5, FVector::ForwardVector, Combat->ServerTime(), 0);
    TestEqual(TEXT("Old time, invalid aim and previous round rejected"), Combat->Ammo.Magazine, FullMagazine - 1);

    Combat->ServerReload_Implementation(1);
    TestTrue(TEXT("Reload state is recoverable"), Combat->ActionState.Action == EPSCombatAction::Reloading &&
        Combat->ActionState.Duration > 0 && Combat->ActionState.Sequence > 0);
    const int32 ReloadSequence = Combat->ActionState.Sequence;
    Combat->ServerReload_Implementation(1);
    TestEqual(TEXT("Repeated reload does not restart action"), Combat->ActionState.Sequence, ReloadSequence);
    Combat->ServerFire_Implementation(6, FVector::ForwardVector, Combat->ServerTime(), 1);
    TestEqual(TEXT("Cannot fire while reloading"), Combat->Ammo.Magazine, FullMagazine - 1);
    const int32 TotalAmmo = Combat->Ammo.Magazine + Combat->Ammo.Reserve;
    Combat->FinishReload();
    Combat->FinishReload();
    TestEqual(TEXT("Reload completes only once"), Combat->Ammo.Magazine, FullMagazine);
    TestEqual(TEXT("Reload conserves ammunition"), Combat->Ammo.Magazine + Combat->Ammo.Reserve, TotalAmmo);

    State->MatchState.Phase = EPSMatchPhase::Freeze;
    Combat->ServerFire_Implementation(7, FVector::ForwardVector, Combat->ServerTime(), 1);
    TestEqual(TEXT("Freeze phase blocks fire"), Combat->Ammo.Magazine, FullMagazine);
    TestEqual(TEXT("Freeze phase blocks damage"), Character->TakeDamage(25, FDamageEvent(), nullptr, nullptr), 0.f);
    State->MatchState.Phase = EPSMatchPhase::Live;
    TestEqual(TEXT("Damage clamps to remaining health"), Character->TakeDamage(999, FDamageEvent(), nullptr, nullptr), 100.f);
    TestFalse(TEXT("Lethal damage kills character"), Character->IsAlive());
    TestTrue(TEXT("Death cancels combat"), Combat->ActionState.Action == EPSCombatAction::Dead);
    Combat->ServerFire_Implementation(8, FVector::ForwardVector, Combat->ServerTime(), 1);
    TestEqual(TEXT("Dead character cannot fire"), Combat->Ammo.Magazine, FullMagazine);
    TestEqual(TEXT("Dead character cannot take damage twice"), Character->TakeDamage(25, FDamageEvent(), nullptr, nullptr), 0.f);
    World->DestroyWorld(false);
    return true;
}

#endif
