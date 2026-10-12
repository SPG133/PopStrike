#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PSWeaponDefinition.generated.h"

class UAnimInstance;
class UAnimMontage;
class UParticleSystem;
class USkeletalMesh;
class USoundBase;
class UTexture2D;

USTRUCT(BlueprintType)
struct POPSTRIKE_API FPSWeaponAnimations
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation") TObjectPtr<UAnimMontage> Fire = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation") TObjectPtr<UAnimMontage> Reload = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation") TObjectPtr<UAnimMontage> Equip = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation") TObjectPtr<UAnimMontage> Unequip = nullptr;
};

// Shared immutable configuration. Live ammunition and action state belong to the actor.
USTRUCT(BlueprintType)
struct POPSTRIKE_API FPSWeaponData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon") FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon") TObjectPtr<UTexture2D> Icon = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ammo", meta=(ClampMin="1", ClampMax="10000")) int32 MagazineCapacity = 30;
    // Excludes the magazine already loaded. Reload uses a pooled reserve, retaining leftover rounds.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ammo", meta=(ClampMin="0", ClampMax="10000")) int32 SpareMagazineCount = 3;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fire", meta=(ClampMin="1.0", ClampMax="6000.0", Units="RPM")) float FireRateRPM = 600.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fire", meta=(ClampMin="0.0")) float Damage = 20.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fire", meta=(ClampMin="0.0", Units="cm")) float Range = 10000.f;
    // Authoritative duration; animation notifies must not complete a reload.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ammo", meta=(ClampMin="0.01", Units="s")) float ReloadDuration = 2.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mesh") TObjectPtr<USkeletalMesh> WeaponMesh = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mesh") TSubclassOf<UAnimInstance> WeaponAnimClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mesh") FName HandSocket = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mesh") FName MuzzleSocket = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mesh") FTransform GripTransform;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation") FPSWeaponAnimations WeaponAnimations;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation") FPSWeaponAnimations CharacterAnimations;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation") FPSWeaponAnimations FirstPersonAnimations;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation") TSubclassOf<UAnimInstance> CharacterAnimationLayer;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effects") TObjectPtr<USoundBase> FireSound = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effects") TObjectPtr<USoundBase> ReloadSound = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effects") TObjectPtr<UParticleSystem> MuzzleEffect = nullptr;
};

UCLASS(BlueprintType)
class POPSTRIKE_API UPSWeaponDefinition : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon") FPSWeaponData Data;
};
