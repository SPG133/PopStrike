#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PSLobbyTypes.h"
#include "PSLobbyGameMode.generated.h"

class APSLobbyPlayerController;

UCLASS()
class POPSTRIKE_API APSLobbyGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    APSLobbyGameMode();

    virtual void PreLogin(
        const FString& Options,
        const FString& Address,
        const FUniqueNetIdRepl& UniqueId,
        FString& ErrorMessage) override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;

    void TrySetTeam(APSLobbyPlayerController* PC, EPSLobbyTeam Team);
    void TryChangeMap(APSLobbyPlayerController* PC, int32 Direction);
    void TryStartMatch(APSLobbyPlayerController* PC);

private:
    bool IsHost(const APSLobbyPlayerController* PC) const;
    void SendMessage(APSLobbyPlayerController* PC, const FString& Message) const;

    int32 NextJoinOrder = 0;
    bool bTraveling = false;
    void HandleTravelFailure(UWorld* World, ETravelFailure::Type Type, const FString& Error);
};
