#include "PSFPSCharacter.h"
#include "PSMatchPlayerState.h"
#include "Net/UnrealNetwork.h"

APSFPSCharacter::APSFPSCharacter()
{
    bReplicates = true;
}

void APSFPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APSFPSCharacter, IsSlowWalking);
}

void APSFPSCharacter::OnStartCrouch(float HeightAdjust, float ScaledHeightAdjust)
{
    Super::OnStartCrouch(HeightAdjust, ScaledHeightAdjust);
    OnIsCrouchingChanged(true);
}

void APSFPSCharacter::OnEndCrouch(float HeightAdjust, float ScaledHeightAdjust)
{
    Super::OnEndCrouch(HeightAdjust, ScaledHeightAdjust);
    OnIsCrouchingChanged(false);
}

void APSFPSCharacter::SetIsSlowWalking(bool bRequested)
{
    if (!HasAuthority())
    {
        if (IsLocallyControlled()) ServerSetIsSlowWalking(bRequested);
        return;
    }
    if (IsSlowWalking == bRequested) return;
    IsSlowWalking = bRequested;
    OnRep_IsSlowWalking();
    ForceNetUpdate();
}

void APSFPSCharacter::ServerSetIsSlowWalking_Implementation(bool bRequested)
{
    SetIsSlowWalking(bRequested);
}

void APSFPSCharacter::OnRep_IsSlowWalking()
{
    OnIsSlowWalkingChanged(IsSlowWalking);
}

void APSFPSCharacter::BeginPlay()
{
    Super::BeginPlay();
    RefreshAppearance();
}

void APSFPSCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    RefreshAppearance();
}

void APSFPSCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    RefreshAppearance();
}

void APSFPSCharacter::RefreshAppearance()
{
    // PlayerState and its appearance may arrive in either order; both paths refresh this event.
    if (!HasActorBegunPlay()) return;
    if (const auto* MatchPlayer = GetPlayerState<APSMatchPlayerState>())
    {
        OnAppearanceChanged(FLinearColor(MatchPlayer->Appearance.PrimaryColor),
            FLinearColor(MatchPlayer->Appearance.SecondaryColor));
    }
}
