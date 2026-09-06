#include "PSLobbyTypes.h"

namespace PSLobby
{
    const TArray<FPSLobbyMapDefinition>& GetMaps()
    {
        static const TArray<FPSLobbyMapDefinition> Maps =
        {
            {
                TEXT("TRAINING YARD"),
                TEXT("近距离三路线爆破训练场"),
                TEXT("/Game/Maps/L_MatchTest"),
                FSoftObjectPath(TEXT("/Game/UI/Maps/T_Map_Training.T_Map_Training"))
            },
            {
                TEXT("FACTORY"),
                TEXT("高低差明显的工业区爆破地图"),
                TEXT("/Game/Maps/L_Factory"),
                FSoftObjectPath(TEXT("/Game/UI/Maps/T_Map_Factory.T_Map_Factory"))
            },
            {
                TEXT("ROOFTOP"),
                TEXT("开放屋顶与室内通道交错的爆破地图"),
                TEXT("/Game/Maps/L_Rooftop"),
                FSoftObjectPath(TEXT("/Game/UI/Maps/T_Map_Rooftop.T_Map_Rooftop"))
            }
        };
        return Maps;
    }

    const FPSLobbyMapDefinition* GetMap(int32 Index)
    {
        const TArray<FPSLobbyMapDefinition>& Maps = GetMaps();
        return Maps.IsValidIndex(Index) ? &Maps[Index] : nullptr;
    }

    int32 WrapMapIndex(int32 Index)
    {
        const int32 Count = GetMaps().Num();
        return Count > 0 ? (Index % Count + Count) % Count : INDEX_NONE;
    }
}
