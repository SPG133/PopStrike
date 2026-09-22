#include "PSMatchPlayerState.h"

#include "PSAvatar.h"
#include "Net/UnrealNetwork.h"

void APSMatchPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APSMatchPlayerState, Team);
    DOREPLIFETIME(APSMatchPlayerState, AvatarBytes);
}

UTexture2D* APSMatchPlayerState::GetAvatar()
{
    if (!AvatarTexture && GetNetMode() != NM_DedicatedServer)
    {
        AvatarTexture = PSAvatar::CreateTexture(AvatarBytes, GetPlayerId());
    }
    return AvatarTexture;
}

void APSMatchPlayerState::OnRep_Avatar()
{
    AvatarTexture = nullptr;
}
