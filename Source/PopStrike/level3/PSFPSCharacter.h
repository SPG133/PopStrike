#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PSFPSCharacter.generated.h"

// Blueprint owns movement settings and material presentation.
UCLASS()
class POPSTRIKE_API APSFPSCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    APSFPSCharacter();

    UPROPERTY(ReplicatedUsing=OnRep_IsSlowWalking, BlueprintReadOnly, Category="Character|State")
    bool IsSlowWalking = false;
    // Crouch uses the inherited IsCrouched, Crouch and UnCrouch nodes and CMC replication.
    UFUNCTION(BlueprintCallable, Category="Character|State") void SetIsSlowWalking(bool bRequested);

    UFUNCTION(BlueprintImplementableEvent, Category="Character|State") void OnIsCrouchingChanged(bool bNewValue);
    UFUNCTION(BlueprintImplementableEvent, Category="Character|State") void OnIsSlowWalkingChanged(bool bNewValue);

    UFUNCTION(BlueprintCallable, Category="Character|Appearance") void RefreshAppearance();
    UFUNCTION(BlueprintImplementableEvent, Category="Character|Appearance")
    void OnAppearanceChanged(FLinearColor PrimaryColor, FLinearColor SecondaryColor);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnStartCrouch(float HeightAdjust, float ScaledHeightAdjust) override;
    virtual void OnEndCrouch(float HeightAdjust, float ScaledHeightAdjust) override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_PlayerState() override;

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION() void OnRep_IsSlowWalking();
    UFUNCTION(Server, Reliable) void ServerSetIsSlowWalking(bool bRequested);
};
