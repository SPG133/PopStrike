#include "PSFPSCharacter.h"

#include "PSCombatComponent.h"
#include "PSMatchGameState.h"
#include "PSMatchPlayerState.h"
#include "PSWeapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

APSFPSCharacter::APSFPSCharacter()
{
    GetCapsuleComponent()->InitCapsuleSize(34.f, 90.f);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
    GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));
    GetMesh()->SetOwnerNoSee(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    // The authoritative muzzle socket must still update on a non-rendering server.
    GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    bUseControllerRotationYaw = true;
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->MaxWalkSpeed = 450.f;
    GetCharacterMovement()->MaxWalkSpeedCrouched = 200.f;
    GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(GetRootComponent());
    // Keep the local view and the server's GetPawnViewLocation at the same height.
    BaseEyeHeight = 64.f;
    Camera->SetRelativeLocation(FVector(0, 0, BaseEyeHeight));
    Camera->bUsePawnControlRotation = true;
    Combat = CreateDefaultSubobject<UPSCombatComponent>(TEXT("Combat"));
    SetNetUpdateFrequency(60.f);
    SetMinNetUpdateFrequency(20.f);
}

void APSFPSCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority()) Health = FMath::Max(1.f, MaxHealth);
    if (auto* State = GetWorld()->GetGameState<APSMatchGameState>())
        State->OnChanged.AddDynamic(this, &ThisClass::UpdateMatchPhase);
    UpdateMatchPhase();
}

void APSFPSCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (auto* State = GetWorld()->GetGameState<APSMatchGameState>())
        State->OnChanged.RemoveDynamic(this, &ThisClass::UpdateMatchPhase);
    Super::EndPlay(EndPlayReason);
}

void APSFPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APSFPSCharacter, Health);
}

bool APSFPSCharacter::CanMove() const
{
    const auto* State = GetWorld()->GetGameState<APSMatchGameState>();
    return IsAlive() && State && (State->MatchState.Phase == EPSMatchPhase::Live ||
        State->MatchState.Phase == EPSMatchPhase::BombPlanted);
}

void APSFPSCharacter::UpdateMatchPhase()
{
    if (CanMove())
    {
        if (GetCharacterMovement()->MovementMode == MOVE_None)
            GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    }
    else
    {
        GetCharacterMovement()->DisableMovement();
        Combat->CancelActions(!IsAlive());
    }
}

void APSFPSCharacter::PawnClientRestart()
{
    Super::PawnClientRestart();
    if (Combat->Weapon) Combat->Weapon->RefreshPresentation();
    UpdateMatchPhase();
}

void APSFPSCharacter::UnPossessed()
{
    Combat->CancelActions(!IsAlive());
    Super::UnPossessed();
}

void APSFPSCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
    Camera->SetRelativeLocation(FVector(0, 0, BaseEyeHeight));
}

void APSFPSCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
    Camera->SetRelativeLocation(FVector(0, 0, BaseEyeHeight));
}

float APSFPSCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority() || !CanMove() || !FMath::IsFinite(DamageAmount) || DamageAmount <= 0) return 0;
    const auto* SourceState = EventInstigator ? EventInstigator->GetPlayerState<APSMatchPlayerState>() : nullptr;
    const auto* TargetState = GetPlayerState<APSMatchPlayerState>();
    if (SourceState && TargetState && SourceState->Team != EPSLobbyTeam::None &&
        SourceState->Team == TargetState->Team) return 0;
    const float Applied = FMath::Min(Health, DamageAmount);
    const float Previous = Health;
    Health -= Applied;
    OnRep_Health(Previous);
    ForceNetUpdate();
    Super::TakeDamage(Applied, DamageEvent, EventInstigator, DamageCauser);
    return Applied;
}

void APSFPSCharacter::OnRep_Health(float PreviousHealth)
{
    if (!IsAlive())
    {
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        GetCharacterMovement()->DisableMovement();
        Combat->CancelActions(true);
    }
    if (GetNetMode() != NM_DedicatedServer) PresentHealth(PreviousHealth);
}
