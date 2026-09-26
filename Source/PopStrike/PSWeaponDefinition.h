#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PSWeaponDefinition.generated.h"

class APSWeapon;
class USkeletalMesh;

/** Shared, immutable weapon tuning. Per-player ammunition belongs to CombatComponent. */
UCLASS(BlueprintType)
class POPSTRIKE_API UPSWeaponDefinition : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon") TSubclassOf<APSWeapon> WeaponClass;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon") TObjectPtr<USkeletalMesh> Mesh;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon", meta=(ClampMin="1")) int32 MagazineSize = 30;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon", meta=(ClampMin="0")) int32 StartingReserve = 90;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon", meta=(ClampMin="0.05")) float FireInterval = 0.1f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon", meta=(ClampMin="0.1")) float ReloadDuration = 2.333333f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon", meta=(ClampMin="0.0")) float Damage = 25.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon", meta=(ClampMin="100.0")) float Range = 15000.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon") FName HandSocket = TEXT("hand_r");
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon") FName MuzzleSocket = TEXT("Muzzle");
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon") FTransform GripTransform;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon") FTransform FirstPersonTransform;
};
