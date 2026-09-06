#pragma once

#include "CoreMinimal.h"

namespace PSConfig
{
    inline constexpr int32 MaxPlayers = 6;
    inline constexpr int32 TeamSize = 3;
    inline constexpr int32 MaxNameLength = 16;

    inline constexpr int32 AvatarSide = 128;
    inline constexpr int32 MaxAvatarBytes = 12 * 1024;
    inline constexpr int64 MaxAvatarFileBytes = 10ll * 1024ll * 1024ll;

    inline constexpr TCHAR MainMenuMap[] = TEXT("/Game/UI/Maps/L_MainMenu");
    inline constexpr TCHAR MainMenuWidget[] = TEXT("/Game/UI/Lobby/WBP_MainMenu");
    inline constexpr TCHAR LobbyWidget[] = TEXT("/Game/UI/Lobby/WBP_Lobby");
}
