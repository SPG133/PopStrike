#pragma once

#include "CoreMinimal.h"
#include "PSLobbyTypes.generated.h"

class UTexture2D;

UENUM(BlueprintType)
enum class EPSLobbyTeam : uint8
{
    None,
    A,
    B
};

USTRUCT(BlueprintType)
struct FPSLobbyMap
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText Name;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText Description;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UWorld> Level;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UTexture2D> Preview;
};
