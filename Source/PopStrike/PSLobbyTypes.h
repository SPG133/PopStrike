#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"
#include "PSLobbyTypes.generated.h"

UENUM(BlueprintType)
enum class EPSLobbyTeam : uint8
{
    None,
    TeamA,
    TeamB
};

struct FPSLobbyMapDefinition
{
    FString Name;
    FString Description;
    FString LevelPackage;
    FSoftObjectPath PreviewTexture;
};

namespace PSLobby
{
    const TArray<FPSLobbyMapDefinition>& GetMaps();
    const FPSLobbyMapDefinition* GetMap(int32 Index);
    int32 WrapMapIndex(int32 Index);
}
