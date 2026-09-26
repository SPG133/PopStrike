#include "PSWeapon.h"

#include "PSCombatComponent.h"
#include "PSFPSCharacter.h"
#include "PSWeaponDefinition.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"

APSWeapon::APSWeapon()
{
    bReplicates = true;
    bNetUseOwnerRelevancy = true;
    WorldMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WorldMesh"));
    SetRootComponent(WorldMesh);
    WorldMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WorldMesh->SetOwnerNoSee(true);
    FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
    FirstPersonMesh->SetupAttachment(WorldMesh);
    FirstPersonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    FirstPersonMesh->SetOnlyOwnerSee(true);
    FirstPersonMesh->SetCastShadow(false);
}

void APSWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APSWeapon, Definition);
}

void APSWeapon::Initialize(UPSWeaponDefinition* InDefinition)
{
    check(HasAuthority());
    Definition = InDefinition;
    RefreshPresentation();
}

void APSWeapon::BeginPlay()
{
    Super::BeginPlay();
    RefreshPresentation();
}

void APSWeapon::OnRep_Owner()
{
    Super::OnRep_Owner();
    RefreshPresentation();
}

void APSWeapon::OnRep_Definition()
{
    RefreshPresentation();
}

void APSWeapon::RefreshPresentation()
{
    auto* Character = Cast<APSFPSCharacter>(GetOwner());
    if (!Character || !Definition) return;
    WorldMesh->SetSkeletalMesh(Definition->Mesh);
    AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        Definition->HandSocket);
    WorldMesh->SetRelativeTransform(Definition->GripTransform);
    FirstPersonMesh->SetSkeletalMesh(Definition->Mesh);
    FirstPersonMesh->AttachToComponent(Character->Camera,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    FirstPersonMesh->SetRelativeTransform(Definition->FirstPersonTransform);
    FirstPersonMesh->SetVisibility(Character->IsLocallyControlled());
    if (Character->Combat) Character->Combat->RefreshPresentation();
}

FVector APSWeapon::GetMuzzleLocation() const
{
    return Definition && WorldMesh->DoesSocketExist(Definition->MuzzleSocket)
        ? WorldMesh->GetSocketLocation(Definition->MuzzleSocket) : GetActorLocation();
}
