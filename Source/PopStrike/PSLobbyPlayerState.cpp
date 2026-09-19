#include "PSLobbyPlayerState.h"

#include "PSLobbyGameState.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "Net/UnrealNetwork.h"

void APSLobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APSLobbyPlayerState, Team);
    DOREPLIFETIME(APSLobbyPlayerState, AvatarBytes);
}

UTexture2D* APSLobbyPlayerState::GetAvatar()
{
    if (!AvatarTexture && GetNetMode() != NM_DedicatedServer)
    {
        if (!AvatarBytes.IsEmpty())
        {
            AvatarTexture = FImageUtils::ImportBufferAsTexture2D(AvatarBytes);
        }
        else
        {
            FRandomStream Random(GetPlayerId());
            const FColor AvatarColor(
                Random.RandRange(64, 255),
                Random.RandRange(64, 255),
                Random.RandRange(64, 255));
            AvatarTexture = UTexture2D::CreateTransient(1, 1, PF_B8G8R8A8);
            if (AvatarTexture)
            {
                uint32* Pixel = static_cast<uint32*>(
                    AvatarTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
                *Pixel = AvatarColor.DWColor();
                AvatarTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
                AvatarTexture->UpdateResource();
            }
        }
    }
    return AvatarTexture;
}

void APSLobbyPlayerState::OnRep_Data()
{
    if (auto* Lobby = GetWorld()->GetGameState<APSLobbyGameState>()) Lobby->BroadcastChanged();
}

void APSLobbyPlayerState::OnRep_Avatar()
{
    AvatarTexture = nullptr;
    OnRep_Data();
}

void APSLobbyPlayerState::OnRep_PlayerName()
{
    Super::OnRep_PlayerName();
    OnRep_Data();
}
