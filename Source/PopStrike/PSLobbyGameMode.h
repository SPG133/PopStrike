#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PSLobbyTypes.h"
#include "PSLobbyGameMode.generated.h"

UCLASS()
class POPSTRIKE_API APSLobbyGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    APSLobbyGameMode();

    UPROPERTY(EditDefaultsOnly, Category="Lobby")
    TArray<FPSLobbyMap> AvailableMaps;

    virtual void InitGameState() override;
    virtual void PreLogin(const FString& Options, const FString& Address,
        const FUniqueNetIdRepl& UniqueId, FString& Error) override;
    virtual void PostLogin(APlayerController* Player) override;
    virtual void Logout(AController* Player) override;
};
