#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PSWeapon.generated.h"

class UPSWeaponDefinition;
class USkeletalMeshComponent;

UCLASS()
class POPSTRIKE_API APSWeapon : public AActor
{
    GENERATED_BODY()
public:
    APSWeapon();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon") TObjectPtr<USkeletalMeshComponent> WorldMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon") TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;
    UPROPERTY(ReplicatedUsing=OnRep_Definition, BlueprintReadOnly, Category="Weapon") TObjectPtr<UPSWeaponDefinition> Definition;
    void Initialize(UPSWeaponDefinition* InDefinition);
    void RefreshPresentation();
    FVector GetMuzzleLocation() const;
    UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category="Weapon|Presentation")
    void PresentShot(bool bFirstPerson);
    UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category="Weapon|Presentation")
    void PresentReload(bool bReloading, float ElapsedSeconds, float Duration);
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnRep_Owner() override;
protected:
    virtual void BeginPlay() override;
private:
    UFUNCTION() void OnRep_Definition();
};
